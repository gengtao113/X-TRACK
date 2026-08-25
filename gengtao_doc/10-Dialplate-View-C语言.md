# 10 Dialplate View：只建 LVGL 控件（C 语言）

源码：`USER/App/Pages/Dialplate/DialplateView.h`、`DialplateView.cpp`

View 的规矩：**只 `lv_obj_create`、设样式、做动画。禁止读 GPS、算速度、Subscribe。**  
Presenter 通过 `View.ui.xxx` 拿到控件指针去改文字、绑事件。

---

## 1. C 里它就是一张控件句柄表

C++ `class DialplateView` 里的成员 `ui` 等价于：

```c
typedef struct {
    lv_obj_t *cont;
    lv_obj_t *label_value;
    lv_obj_t *label_unit;
} SubInfo_t;                 /* 一组：数值 + 单位 */

typedef struct {
    struct {
        lv_obj_t *cont;
        lv_obj_t *label_speed;
        lv_obj_t *label_unit;    /* 源码拼写 labelUint */
    } top;

    struct {
        lv_obj_t *cont;
        SubInfo_t grp[4];        /* AVG / Time / Trip / Calorie */
    } bottom;

    struct {
        lv_obj_t *cont;
        lv_obj_t *btn_map;
        lv_obj_t *btn_rec;
        lv_obj_t *btn_menu;
    } btn;

    lv_anim_timeline_t *anim_timeline;
} DialplateView;

void DialplateView_Create(DialplateView *v, lv_obj_t *root);
void DialplateView_Delete(DialplateView *v);
void DialplateView_AppearAnimStart(DialplateView *v, int reverse);
```

`View.Create(_root)` = `DialplateView_Create(&view, page->root)`。  
`_root` 是 PageManager 给这一页准备的全屏容器，所有控件都挂在它下面。

---

## 2. Create：先拼控件，再根据最终坐标做入场动画

```c
void DialplateView_Create(DialplateView *v, lv_obj_t *root)
{
    bottom_create(v, root);   /* 先底部，按钮要相对它对齐 */
    top_create(v, root);
    btn_create(v, root);

    /* 记下「动画结束时」的位置/高度 */
    lv_coord_t y_top = lv_obj_get_y(v->top.cont);
    lv_coord_t y_bot = lv_obj_get_y(v->bottom.cont);
    lv_coord_t h_btn = lv_obj_get_height(v->btn.btn_rec);

    v->anim_timeline = lv_anim_timeline_create();
    /* 时间轴：上半从屏幕外滑入 -> 下半滑入并淡入 -> 三个按钮高度从 0 展开 */
    lv_anim_timeline_add(...);
}
```

源码顺序就是 `BottomInfo_Create` → `TopInfo_Create` → `BtnCont_Create`，因为按钮容器 `align_to(bottomInfo.cont, OUT_BOTTOM)`。

动画 **只登记，不在 Create 里播放**。真正 `lv_anim_timeline_start` 发生在 Presenter 的 `onViewWillAppear`。这样切页时控件已经在，动画从隐藏位置播到目标位置。

时间轴（毫秒）：

| 起点 | 对象 | 效果 |
|------|------|------|
| 0 | 上半 cont | y：从负高度滑到目标 y |
| 200 | 下半 cont | y 滑入 + 透明度 0→覆盖 |
| 500 / 600 / 700 | 地图 / 录轨 / 菜单 | height：0 → 按钮高度 |

宏 `ANIM_DEF` 只是给 `lv_anim_timeline_wrapper_t` 填一行，C 里写成结构体数组即可。

---

## 3. 上半：大号时速

`TopInfo_Create`：

```c
static void top_create(DialplateView *v, lv_obj_t *par)
{
    lv_obj_t *cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, 142);          /* 全宽 142 高 */
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(cont, 27, 0);
    lv_obj_set_y(cont, -36);                         /* 往上探出一点，圆角露出 */
    v->top.cont = cont;

    lv_obj_t *lab = lv_label_create(cont);
    lv_obj_set_style_text_font(lab, ResourcePool_GetFont("bahnschrift_65"), 0);
    lv_obj_set_style_text_color(lab, lv_color_white(), 0);
    lv_label_set_text(lab, "00");                    /* 占位，Presenter 再改 */
    lv_obj_align(lab, LV_ALIGN_TOP_MID, 0, 63);
    v->top.label_speed = lab;

    lab = lv_label_create(cont);
    lv_obj_set_style_text_font(lab, ResourcePool_GetFont("bahnschrift_17"), 0);
    lv_label_set_text(lab, "km/h");
    lv_obj_align_to(lab, v->top.label_speed, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    v->top.label_unit = lab;
}
```

这里 **写死 "00"**，不读 Model。View 不知道当前速度。

`ResourcePool::GetFont("bahnschrift_65")` 是按名字取字体，C 就是资源表查找。

---

## 4. 下半：四组副信息

容器全宽 90 高，放在 y=106，横向 flex 均分：

```c
static void bottom_create(DialplateView *v, lv_obj_t *par)
{
    lv_obj_t *cont = lv_obj_create(par);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, 90);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 106);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    v->bottom.cont = cont;

    static const char *units[4] = { "AVG", "Time", "Trip", "Calorie" };
    int i;
    for (i = 0; i < 4; i++)
        subinfo_create(cont, &v->bottom.grp[i], units[i]);
}
```

每一组 93×39，纵向 flex：上面数值 Label（先不 set_text，Presenter 填），下面灰色单位：

```c
static void subinfo_create(lv_obj_t *par, SubInfo_t *info, const char *unit)
{
    lv_obj_t *cont = lv_obj_create(par);
    lv_obj_set_size(cont, 93, 39);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    /* ... */

    info->label_value = lv_label_create(cont);   /* 17 号白字 */
    info->label_unit  = lv_label_create(cont);   /* 13 号灰字 */
    lv_label_set_text(info->label_unit, unit);
    info->cont = cont;
}
```

Presenter 的 `Update()` 按下标写：

- `grp[0]` 平均速度  
- `grp[1]` 单次时间  
- `grp[2]` 单次里程 km  
- `grp[3]` 卡路里  

View 不规定含义，只提供四个空 Label。

---

## 5. 三个按钮

按钮条高 40，贴在底部容器下面。三个按钮用同一套 `Btn_Create`，x 偏移 -80 / 0 / 80：

```c
v->btn.btn_map  = btn_create(cont, ResourcePool_GetImage("locate"), -80);
v->btn.btn_rec  = btn_create(cont, ResourcePool_GetImage("start"),     0);
v->btn.btn_menu = btn_create(cont, ResourcePool_GetImage("menu"),     80);
```

单个按钮：

```c
static lv_obj_t *btn_create(lv_obj_t *par, const void *img, lv_coord_t x_ofs)
{
    lv_obj_t *obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 40, 31);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(obj, LV_ALIGN_CENTER, x_ofs, 0);
    lv_obj_set_style_bg_img_src(obj, img, 0);

    lv_obj_set_style_bg_color(obj, lv_color_hex(0x666666), 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xbbbbbb), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff931e), LV_STATE_FOCUSED);
    lv_obj_set_style_width(obj, 45, LV_STATE_PRESSED);
    lv_obj_set_style_height(obj, 25, LV_STATE_PRESSED);
    lv_obj_set_style_radius(obj, 9, 0);

    /* 按下/聚焦时宽高过渡 200ms */
    lv_obj_set_style_transition(obj, &tran, LV_STATE_PRESSED);
    lv_obj_set_style_transition(obj, &tran, LV_STATE_FOCUSED);
    return obj;
}
```

View **不** `lv_obj_add_event_cb`。绑点击在 Presenter 的 `AttachEvent`。  
View 也不改录轨图标；Presenter `SetBtnRecImgSrc("pause")` 直接改 `btn_rec` 的背景图。

`tran` 是 `static` 的 transition 描述符，三个按钮共用一份，C 里放文件作用域即可。

---

## 6. Delete 和 AppearAnimStart

```c
void DialplateView_Delete(DialplateView *v)
{
    if (v->anim_timeline) {
        lv_anim_timeline_del(v->anim_timeline);
        v->anim_timeline = NULL;
    }
    /* 不 lv_obj_del 各个控件：它们挂在 page->root 上，
     * PageManager UNLOAD 时会拆掉整棵树 */
}

void DialplateView_AppearAnimStart(DialplateView *v, int reverse)
{
    lv_anim_timeline_set_reverse(v->anim_timeline, reverse);
    lv_anim_timeline_start(v->anim_timeline);
}
```

C++ 默认参数 `reverse = false`：不写第二实参就是正向入场。源码离场时有一行注释掉的 `AppearAnimStart(true)`，当前没用。

---

## 7. View 禁止做什么

| 不要 | 为什么 |
|------|--------|
| `HAL_GPS_GetInfo` | 硬件只进 DataProc |
| `Account_Subscribe` | 那是 Model 的事 |
| `lv_timer_create` 刷速度 | 切页停表由 Presenter 生命周期管 |
| `lv_group_add_obj` | 输入焦点属于「页面出现/消失」，在 Presenter |

Presenter 改文字示例（C）：

```c
lv_label_set_text_fmt(d->view.top.label_speed, "%02d", (int)speed);
```

对应 `View.ui.topInfo.labelSpeed`。

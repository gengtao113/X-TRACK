# 12 Dialplate Presenter：生命周期 + 交互（C 语言）

源码：`USER/App/Pages/Dialplate/Dialplate.h`、`Dialplate.cpp`

Presenter 粘合 View 和 Model：PageManager 回调里创建/销毁，处理按键和录轨 UI 状态机。  
**不读 GPS 寄存器，不 `lv_obj_create` 整页布局**（布局在 View）。

View / Model 细节见：

- [10-Dialplate-View-C语言.md](10-Dialplate-View-C语言.md)
- [11-Dialplate-Model-C语言.md](11-Dialplate-Model-C语言.md)

---

## 1. C 结构体：页面 = Page 头 + View + Model

```c
typedef enum {
    RECORD_STATE_READY,
    RECORD_STATE_RUN,
    RECORD_STATE_PAUSE,
    RECORD_STATE_STOP
} RecordState_t;

typedef struct {
    Page           base;        /* 必须第一项，继承 PageBase */
    DialplateView  view;
    DialplateModel model;
    lv_timer_t    *timer;       /* 1s 刷数，不是 Account 的 timer */
    RecordState_t  rec_state;
    lv_obj_t      *last_focus;  /* 离开页时记住编码器焦点 */
} DialplatePage;
```

C++ `class Dialplate : public PageBase` 就是上面这个布局。  
`virtual onViewLoad` = 填进 `PageOps` 的函数指针，由 PageManager 调用。

构造：`recState = READY`，`lastFocus = NULL`。

`onCustomAttrConfig`：`SetCustomLoadAnimType(LOAD_ANIM_NONE)`，切到表盘不要整页滑动（开机 Replace 过来更干净）。

---

## 2. 生命周期：资源成对开关

```c
static void dial_on_load(Page *page)
{
    DialplatePage *d = (DialplatePage *)page;

    DialplateModel_Init(&d->model);
    DialplateView_Create(&d->view, page->root);

    lv_obj_add_event_cb(d->view.btn.btn_map,  on_event, LV_EVENT_ALL, d);
    lv_obj_add_event_cb(d->view.btn.btn_rec,  on_event, LV_EVENT_ALL, d);
    lv_obj_add_event_cb(d->view.btn.btn_menu, on_event, LV_EVENT_ALL, d);
}

static void dial_on_will_appear(Page *page)
{
    DialplatePage *d = (DialplatePage *)page;
    lv_group_t *group = lv_group_get_default();

    lv_indev_wait_release(lv_indev_get_act());  /* 吃掉切页时还按着的键 */
    lv_group_set_wrap(group, false);            /* 编码器到头不循环，减少误触 */

    lv_group_add_obj(group, d->view.btn.btn_map);
    lv_group_add_obj(group, d->view.btn.btn_rec);
    lv_group_add_obj(group, d->view.btn.btn_menu);
    lv_group_focus_obj(d->last_focus ? d->last_focus : d->view.btn.btn_rec);

    DialplateModel_SetStatusBarStyle(&d->model, STATUS_BAR_STYLE_TRANSP);
    dial_update(d);                             /* 先刷一帧，避免 00 停很久 */
    DialplateView_AppearAnimStart(&d->view, 0);
}

static void dial_on_did_appear(Page *page)
{
    DialplatePage *d = (DialplatePage *)page;
    d->timer = lv_timer_create(on_timer_update, 1000, d);
}

static void dial_on_will_disappear(Page *page)
{
    DialplatePage *d = (DialplatePage *)page;
    lv_group_t *group = lv_group_get_default();

    d->last_focus = lv_group_get_focused(group);
    lv_group_remove_all_objs(group);
    lv_timer_del(d->timer);
    d->timer = NULL;
}

static void dial_on_unload(Page *page)
{
    DialplatePage *d = (DialplatePage *)page;
    DialplateModel_Deinit(&d->model);
    DialplateView_Delete(&d->view);
}
```

和源码回调一一对应：

| PageManager | 本页动作 |
|-------------|----------|
| onViewLoad | Model.Init、View.Create、绑三个按钮 |
| onViewWillAppear | 编码器 group、透明状态栏、Update、入场动画 |
| onViewDidAppear | 开 1s timer |
| onViewWillDisappear | 存焦点、清空 group、**删 timer** |
| onViewUnload | Model.Deinit、View.Delete |

空着的 `onViewDidLoad` / `DidDisappear` / `DidUnload` 可以不写。

切到地图若不删 timer，还会 `Update()` 去改可能已卸载的 Label。这是 Presenter 存在的主要理由。

`on_timer_update` 必须是 C 回调：

```c
static void on_timer_update(lv_timer_t *t)
{
    DialplatePage *d = t->user_data;
    dial_update(d);
}
```

C++ `static void onTimerUpdate` 同样没有 `this`，用 `timer->user_data` 找回页面。

---

## 3. Update：Model 缓存 → View Label

```c
static void dial_update(DialplatePage *d)
{
    char buf[16];
    SportStatus_Info_t *s = &d->model.sport;

    lv_label_set_text_fmt(d->view.top.label_speed, "%02d", (int)s->speedKph);

    lv_label_set_text_fmt(d->view.bottom.grp[0].label_value, "%0.1f km/h", s->speedAvgKph);
    lv_label_set_text(d->view.bottom.grp[1].label_value,
                      MakeTimeString(s->singleTime, buf, sizeof(buf)));
    lv_label_set_text_fmt(d->view.bottom.grp[2].label_value, "%0.1f km",
                          s->singleDistance / 1000.0f);
    lv_label_set_text_fmt(d->view.bottom.grp[3].label_value, "%d k",
                          (int)s->singleCalorie);
}
```

源码 `Model.GetSpeed()` 就是 `sportStatusInfo.speedKph`。  
时间字符串用 `DataProc::MakeTimeString`。View 不参与格式化业务含义，只提供 Label。

---

## 4. 按键：一个 C 回调分发

```c
static void on_event(lv_event_t *e)
{
    DialplatePage *d = lv_event_get_user_data(e);
    lv_obj_t *obj = lv_event_get_current_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED)
        on_btn_clicked(d, obj);

    if (obj == d->view.btn.btn_rec) {
        if (code == LV_EVENT_SHORT_CLICKED)
            on_record(d, 0);
        else if (code == LV_EVENT_LONG_PRESSED)
            on_record(d, 1);
    }
}

static void on_btn_clicked(DialplatePage *d, lv_obj_t *btn)
{
    if (btn == d->view.btn.btn_map)
        PageManager_Push(d->base.manager, "Pages/LiveMap");
    else if (btn == d->view.btn.btn_menu)
        PageManager_Push(d->base.manager, "Pages/SystemInfos");
}
```

对应 `_Manager->Push("Pages/LiveMap")`。录轨键短按不走 Push，只进 `on_record`。

`lv_event_get_user_data` 就是绑事件时传入的 `d`。C++ `static onEvent` 同样要这一步。

---

## 5. 录轨 UI 状态机（只存在 Presenter）

这是界面状态，不是 GPX 文件状态。文件由 Model.Notify Recorder 去改。

```
          长按且 GPS 就绪
 READY ------------------> RUN
  ^                         |
  | 长按 STOP               | 短按暂停
  |                         v
 STOP <----- 长按确认 ---- PAUSE
              短按则继续回 RUN
```

```c
static void on_record(DialplatePage *d, int long_press)
{
    switch (d->rec_state) {
    case RECORD_STATE_READY:
        if (long_press) {
            if (!DialplateModel_GpsReady(&d->model)) {
                DialplateModel_PlayMusic(&d->model, "Error");
                return;   /* 状态不变 */
            }
            DialplateModel_PlayMusic(&d->model, "Connect");
            DialplateModel_RecorderCmd(&d->model, REC_START);
            set_rec_img(d, "pause");
            d->rec_state = RECORD_STATE_RUN;
        }
        break;

    case RECORD_STATE_RUN:
        if (!long_press) {
            DialplateModel_PlayMusic(&d->model, "UnstableConnect");
            DialplateModel_RecorderCmd(&d->model, REC_PAUSE);
            set_rec_img(d, "start");
            d->rec_state = RECORD_STATE_PAUSE;
        }
        break;

    case RECORD_STATE_PAUSE:
        if (long_press) {
            DialplateModel_PlayMusic(&d->model, "NoOperationWarning");
            set_rec_img(d, "stop");
            DialplateModel_RecorderCmd(&d->model, REC_READY_STOP);
            d->rec_state = RECORD_STATE_STOP;
        } else {
            DialplateModel_PlayMusic(&d->model, "Connect");
            DialplateModel_RecorderCmd(&d->model, REC_CONTINUE);
            set_rec_img(d, "pause");
            d->rec_state = RECORD_STATE_RUN;
        }
        break;

    case RECORD_STATE_STOP:
        if (long_press) {
            DialplateModel_PlayMusic(&d->model, "Disconnect");
            DialplateModel_RecorderCmd(&d->model, REC_STOP);
            set_rec_img(d, "start");
            d->rec_state = RECORD_STATE_READY;
        } else {
            DialplateModel_PlayMusic(&d->model, "Connect");
            DialplateModel_RecorderCmd(&d->model, REC_CONTINUE);
            set_rec_img(d, "pause");
            d->rec_state = RECORD_STATE_RUN;
        }
        break;
    }
}

static void set_rec_img(DialplatePage *d, const char *name)
{
    lv_obj_set_style_bg_img_src(d->view.btn.btn_rec,
                                ResourcePool_GetImage(name), 0);
}
```

| 状态 | 短按 | 长按 |
|------|------|------|
| READY | 无 | GPS 好：开始录；不好：Error 音 |
| RUN | 暂停 | （源码不处理长按） |
| PAUSE | 继续 | 进入待停止，图标 stop |
| STOP | 继续录 | 真正 STOP，回到 READY |

产品上「长按中间键两声嘟」就是 READY 时长按但 `GetGPSReady()` 为假。

---

## 6. 三层如何接在一起

```
PageManager 调 ops.on_load
    DialplateModel_Init          -> 总线账号
    DialplateView_Create         -> 控件树
    add_event_cb(..., d)

SportStatus Publish
    model_on_event memcpy sport

lv_timer 1s
    dial_update：sport -> label

用户长按 rec
    on_record：GpsReady / RecorderCmd / 改图标 / rec_state
```

| 文件 | C 模块 | 允许 |
|------|--------|------|
| DialplateView.c | 控件 | `lv_obj_create`、样式、动画 |
| DialplateModel.c | 总线 | Subscribe/Pull/Notify、缓存 sport |
| Dialplate.c | 页面 | 生命周期、group、timer、状态机、Push |

---

## 7. C++ → C 对照（本页）

| C++ | C |
|-----|---|
| `class Dialplate : public PageBase` | `struct { Page base; ...}` |
| `View` / `Model` 成员 | 内嵌两个结构体 |
| `this` 传入 timer/event | `user_data` |
| `static onEvent` | 文件内 `static void on_event` |
| `_Manager->Push("Pages/LiveMap")` | `PageManager_Push(mgr, "Pages/LiveMap")` |
| `virtual onViewLoad` | `ops->on_load` |

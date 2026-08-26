# 18 MVP 三文件：设计思想与实现

对应学习路径第 3 条。表盘三个文件的控件/总线细节见文档 10 / 11 / 12。  
本文回答：**为什么要拆开，以及一次按键如何穿过三层。**

源码样板：`Pages/Dialplate/`。复制起点：`Pages/_Template/`。

---

## 1. 设计要解决什么

单文件页面常见写法：读 GPS、算均速、`lv_label_set_text`、切页，全在一个 `switch` 或一个 class 里。

换屏（分辨率、字体）要改同一文件里的业务；换 GPS 模块要改同一文件里的按钮逻辑。无法单独测「录轨命令对不对」。

MVP 在本仓库的落地很具体，不是空谈架构：

| 角色 | 文件 | 允许 | 禁止 |
|------|------|------|------|
| View | `*View.cpp` | `lv_obj_create`、样式、动画 | Subscribe、HAL、算速度 |
| Model | `*Model.cpp` | Account 的 Subscribe/Pull/Notify | `lv_obj_*` |
| Presenter | `*.cpp`（页面类） | 生命周期、按键状态机、把 Model 数填进 View | 直接读寄存器、整页布局 |

强制靠目录和代码评审，编译器不会拦 `View` 里 `#include HAL.h`。约定比语法更重要。

---

## 2. 核心思想（三个结构体，禁止循环调用）

C 对应：

```c
/* view.c 只碰 LVGL */
typedef struct { lv_obj_t *label_speed; lv_obj_t *btn_map; ... } DialplateView;
void DialplateView_Create(DialplateView *v, lv_obj_t *root);
void DialplateView_Delete(DialplateView *v);

/* model.c 只碰总线 */
typedef struct { Account *account; SportStatus_Info_t sport; } DialplateModel;
void DialplateModel_Init(DialplateModel *m);
float DialplateModel_GetSpeed(DialplateModel *m);

/* presenter.c 粘合 */
typedef struct {
    Page base;             /* 必须第一项，继承 PageBase */
    DialplateView view;
    DialplateModel model;
    lv_timer_t *timer;
    int rec_state;
} DialplatePage;
```

依赖方向只能是：

```
Presenter → View
Presenter → Model
View  ↛ Model
Model ↛ View
```

Presenter 继承 `PageBase`，所以 **PageManager 只跟 Presenter 说话**。View/Model 不知道页面栈。

---

## 3. 实现落地（表盘）

### 3.1 谁创建谁销毁

```
onViewLoad
    Model.Init()           开账号、Subscribe
    View.Create(_root)     建所有控件，登记动画（不播放）
    AttachEvent(三个按钮)  user_data = this（Presenter）

onViewWillAppear
    编码器 group 加上三个按钮
    Model.SetStatusBarStyle(...)
    Update()               Model → Label
    View.AppearAnimStart()

onViewDidAppear
    timer = 1s → Update()

onViewWillDisappear
    记焦点、摘 group、删 timer

onViewUnload
    Model.Deinit()
    View.Delete()          只删 timeline；root 由 PageManager 删
```

控件句柄放在 `View.ui`，Presenter 用 `View.ui.topInfo.labelSpeed` 改字，**不在 View 里定时刷**。刷数是 Presenter 的职责，这样离开页时停 timer 一定停得掉。

### 3.2 数据只进 Model

SportStatus Publish → Model.onEvent → `sportStatusInfo`。  
`GetSpeed()` 是 inline，读这块缓存。  
GPS 就绪、录轨、提示音、状态栏样式全部 `Pull`/`Notify`，Presenter 只调 `Model.Xxx()`。

### 3.3 界面状态 vs 业务状态

`recState`（READY/RUN/PAUSE/STOP）在 Presenter。  
文件开停在 Recorder 节点。  
`REC_READY_STOP` 只改状态栏文字，不 Notify Recorder——这是 UI 多出来的一步，不能放进 Model 当「业务真理」，否则状态栏和文件会耦死。

---

## 4. 跟一次长按录轨

```
用户长按录轨键
  View：只是一个 lv_obj，事件已在 Load 时挂到 Presenter::onEvent
  Presenter::onEvent
      static，从 user_data 取回 this
      SHORT_CLICKED → onBtnClicked（录轨键对不上地图/菜单）
      若 obj==btnRec 且 LONG_PRESSED → onRecord(true)
  Presenter::onRecord(READY)
      Model.GetGPSReady()            → Pull GPS
      失败：Model.PlayMusic("Error") → Notify MusicPlayer，return
      成功：PlayMusic("Connect")
            RecorderCommand(START)   → Notify Recorder + StatusBar "REC"
            SetBtnRecImgSrc("pause") → 改 View 按钮背景图
            recState = RUN
```

穿过三层：事件在 View 控件上，决策在 Presenter，副作用在 Model，图标再回到 View。  
没有一层同时知道「长按」和「GPX 怎么写」。

短按地图更短：`onBtnClicked` → `_Manager->Push("Pages/LiveMap")`。View 不知道页面名，Model 也不知道。

---

## 5. 换件时改哪一层

| 变化 | 改哪里 | 不动哪里 |
|------|--------|----------|
| 时速字体/圆角 | View | Model、onRecord |
| 换 GPS 芯片 | HAL + `DP_GPS.cpp` | View、录轨按键逻辑 |
| 录轨要二次确认 | Presenter 状态机 | View 布局、Recorder 写文件 |
| 表盘 1s 改 500ms | Presenter DidAppear 的 timer | Model |
| 新加一页 | 复制 `_Template`，工厂 MATCH，Install，某页 Push | 不必改 DataCenter 核心 |

`_Template` 示范了框架，不是业务：Model 只有 `lv_tick_get()`，证明 Model 可以极瘦；正式页应在 Model 里开 Account。

---

## 6. 新页固定步骤

1. 复制 `Pages/_Template`，改类名与文件名  
2. `AppFactory.cpp` 增加 include 和 `APP_CLASS_MATCH`  
3. `App.cpp`：`Install("类名", "Pages/名字")`  
4. Model::Init 里 `new Account` 并 Subscribe  
5. 需要新数据：`DP_LIST.inc` 加节点，不要在 View 读 HAL  
6. 从已有 Presenter `_Manager->Push("Pages/名字")`

---

## 7. 对照阅读

1. `_Template/` 三文件 — 空壳合同  
2. `Dialplate.h` — Presenter 成员：View、Model、timer、recState  
3. `DialplateView.cpp` 的 Create — 无 HAL  
4. `DialplateModel.cpp` 的 Init/onEvent — 无 lv_obj  
5. `Dialplate.cpp` 的 onViewLoad 与 onRecord  
6. 文档 10 / 11 / 12 — 逐函数  
7. 文档 16 — 生命周期何时调到这三层

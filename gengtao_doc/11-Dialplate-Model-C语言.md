# 11 Dialplate Model：只和 DataCenter 说话（C 语言）

源码：`USER/App/Pages/Dialplate/DialplateModel.h`、`DialplateModel.cpp`

Model 的规矩：**只 Subscribe / Pull / Notify，禁止 `lv_obj_*`。**  
它是表盘在总线上的账号，缓存一份运动数据给 Presenter 显示。

总线四种动作见 [09-总线四句话-对照源码.md](09-总线四句话-对照源码.md)。

---

## 1. C 结构体

```c
typedef enum {
    REC_START = 0,      /* 与 Recorder 节点命令数值相同 */
    REC_PAUSE,
    REC_CONTINUE,
    REC_STOP,
    REC_READY_STOP      /* 只改状态栏显示 STOP，还不关文件 */
} RecCmd_t;

typedef struct {
    SportStatus_Info_t sport;   /* 缓存，对应 sportStatusInfo */
    Account *account;           /* 总线上的句柄，private */
} DialplateModel;

void DialplateModel_Init(DialplateModel *m);
void DialplateModel_Deinit(DialplateModel *m);
int  DialplateModel_GpsReady(DialplateModel *m);
float DialplateModel_GetSpeed(DialplateModel *m);      /* return m->sport.speedKph */
float DialplateModel_GetAvgSpeed(DialplateModel *m);
void DialplateModel_RecorderCmd(DialplateModel *m, RecCmd_t cmd);
void DialplateModel_PlayMusic(DialplateModel *m, const char *name);
void DialplateModel_SetStatusBarStyle(DialplateModel *m, int style);
```

C++ 里 `GetSpeed()` 写在头文件中 = inline。`account` 是 private：Presenter 不能直接 `Model.account->Pull`，必须走封装函数。

`new Account(..., 0, this)` 第三参数缓冲为 0：Model **不 Publish**，不需要乒乓缓存，只当订阅者。

---

## 2. Init / Deinit

```c
static int model_on_event(Account *acc, int event, Account *from,
                          void *data, uint32_t size);

void DialplateModel_Init(DialplateModel *m)
{
    memset(&m->sport, 0, sizeof(m->sport));
    m->account = Account_Create("DialplateModel", center, 0, m);
    /* user = m，回调里还原 */

    Account_Subscribe(m->account, "SportStatus");
    Account_Subscribe(m->account, "Recorder");
    Account_Subscribe(m->account, "StatusBar");
    Account_Subscribe(m->account, "GPS");
    Account_Subscribe(m->account, "MusicPlayer");
    Account_SetCallback(m->account, model_on_event);
}

void DialplateModel_Deinit(DialplateModel *m)
{
    if (m->account) {
        Account_Destroy(m->account);   /* 自动 Unsubscribe */
        m->account = NULL;
    }
}
```

对应 `new Account("DialplateModel", DataProc::Center(), 0, this)`。  
名字 `"DialplateModel"` 在中心必须唯一；页面卸载不 Deinit 会泄漏账号。

为什么五个 Subscribe 都要：

| 名字 | 用途 |
|------|------|
| SportStatus | 收 Publish，更新 `sport` 缓存 |
| GPS | Pull 问卫星数 |
| Recorder | Notify 开始/暂停/停止 |
| StatusBar | Notify 改 REC 标签、透明样式 |
| MusicPlayer | Notify 播提示音 |

**必须先 Subscribe 再 Pull/Notify**，否则 `RES_NOT_FOUND`。

---

## 3. 收 Publish：唯一的 on_event

C++ 的 `static int onEvent` = 没有 `this` 的 C 函数，才能挂到 Account。

```c
static int model_on_event(Account *acc, int event, Account *from,
                          void *data, uint32_t size)
{
    DialplateModel *m = (DialplateModel *)acc->user;

    if (event != EVENT_PUBLISH)
        return -3;   /* 不处理 Pull/Notify/Timer */

    if (strcmp(from->id, "SportStatus") != 0)
        return -8;
    if (size != sizeof(SportStatus_Info_t))
        return -2;

    memcpy(&m->sport, data, size);   /* 立刻拷走，data 指向乒乓读区 */
    return 0;
}
```

源码用 `param->tran->ID` 判断发送方。GPS 也会 Publish，但本回调直接丢掉（不是 SportStatus）。速度刷新不靠 GPS 包，靠 SportStatus 算完再推。

Presenter **没有**在这个回调里改 Label。显示在 1 秒 timer 里读 `m->sport`。这样 LVGL 刷新和总线回调解耦。

---

## 4. Pull：有没有星

```c
int DialplateModel_GpsReady(DialplateModel *m)
{
    GPS_Info_t gps;
    if (Account_Pull(m->account, "GPS", &gps, sizeof(gps)) != 0)
        return 0;
    return gps.satellites > 0;
}
```

对应 `GetGPSReady()`。这是 **现货**：GPS 节点 `EVENT_PULL` 里当场 `HAL_GPS_GetInfo`。  
长按录轨用这个，不能用可能过期的 Publish 缓存。

---

## 5. Notify：录轨命令 + 状态栏

```c
void DialplateModel_RecorderCmd(DialplateModel *m, RecCmd_t cmd)
{
    if (cmd != REC_READY_STOP) {
        Recorder_Info_t rec;
        memset(&rec, 0, sizeof(rec));
        rec.cmd = (int)cmd;     /* READY_STOP 不发给 Recorder */
        rec.time = 1000;
        Account_Notify(m->account, "Recorder", &rec, sizeof(rec));
    }

    StatusBar_Info_t bar;
    memset(&bar, 0, sizeof(bar));
    bar.cmd = STATUS_BAR_CMD_SET_LABEL_REC;

    switch (cmd) {
    case REC_START:
    case REC_CONTINUE:
        bar.param.label_rec.show = 1;
        bar.param.label_rec.str  = "REC";
        break;
    case REC_PAUSE:
        bar.param.label_rec.show = 1;
        bar.param.label_rec.str  = "PAUSE";
        break;
    case REC_READY_STOP:
        bar.param.label_rec.show = 1;
        bar.param.label_rec.str  = "STOP";
        break;
    case REC_STOP:
        bar.param.label_rec.show = 0;
        break;
    }
    Account_Notify(m->account, "StatusBar", &bar, sizeof(bar));
}
```

`DATA_PROC_INIT_STRUCT` = `memset` 0。  
Model **不写 GPX、不改状态栏控件**，只发结构体。`REC_READY_STOP` 是 UI 二次确认：先让状态栏显示 STOP，用户再长按才 `REC_STOP`。

播声音、改状态栏样式同样是 Notify：

```c
void DialplateModel_PlayMusic(DialplateModel *m, const char *name)
{
    MusicPlayer_Info_t info = {0};
    info.music = name;          /* "Error" / "Connect" / ... */
    Account_Notify(m->account, "MusicPlayer", &info, sizeof(info));
}

void DialplateModel_SetStatusBarStyle(DialplateModel *m, int style)
{
    StatusBar_Info_t info = {0};
    info.cmd = STATUS_BAR_CMD_SET_STYLE;
    info.param.style = style;   /* 表盘用 TRANSP 透明 */
    Account_Notify(m->account, "StatusBar", &info, sizeof(info));
}
```

---

## 6. Presenter 怎么用 Model（Model 不知道 Label）

```c
/* 每秒 */
speed = DialplateModel_GetSpeed(&d->model);
lv_label_set_text_fmt(d->view.top.label_speed, "%02d", (int)speed);

/* 长按 */
if (!DialplateModel_GpsReady(&d->model)) {
    DialplateModel_PlayMusic(&d->model, "Error");
    return;
}
DialplateModel_RecorderCmd(&d->model, REC_START);
```

Model 头文件里的 `sportStatusInfo` 是 public，Presenter 也直接读 `singleTime` / `singleDistance`。C 里可以保留这块 public 缓存，或全部改成 getter。

---

## 7. Model 禁止做什么

| 不要 | 应交给谁 |
|------|----------|
| `lv_label_set_text` | Presenter |
| `lv_obj_create` | View |
| `HAL_GPS_GetInfo` | DP_GPS |
| `fopen` 写 GPX | DP_Recorder |
| READY/RUN/PAUSE/STOP 状态机 | Presenter 的 `recState` |

# 20 Pages 从 C++ 转到 C：按步骤做

目标：把 `USER/App/Pages/` 下用 class 写的页面，改成 **结构体 + 函数 + 函数指针表**，语法是 C，行为与现在一致。

本仓库页面仍挂在 C++ 的 `PageManager` / `PageBase` 上。**只改 Pages、不改调度器，C 文件无法 `class Dialplate : public PageBase`。** 必须先换接入方式，再逐页改成 `.c`。

请严格按下面 **步骤 1 → 步骤 7** 做。每一步结束都用 `./gengtao_build.sh run` 验收，通过了再进下一步。不要跳步去改 LiveMap。

设计对照：[16-设计-页面生命周期.md](16-设计-页面生命周期.md)、[18-设计-MVP分层.md](18-设计-MVP分层.md)。  
C++ 语法对照：[06-页面框架-C语言说明.md](06-页面框架-C语言说明.md)。

**不推荐：** 页面改成 `.c`，调度器仍用 `virtual`，再用一个 `PageCAdapter` 去转。能跑，但多一层，后面还得删。

工作方式：在 git 分支上改（例如 `port-pages-c`），不要和正在看的 LinuxSDL2 学习环境缠在一起。原 `.cpp` 改成 `.c` 后删掉对应 `.cpp`，避免重复定义。

---

## 总览

| 步骤 | 做什么 | 验收 |
|------|--------|------|
| 1 | 调度器改调 `PageOps`；现有页面机械填表，**仍是 .cpp，不改业务** | 开机 → 表盘 → 系统信息 → 返回 |
| 2 | `_Template` 改成第一份真正的 `.c`，工厂加一行 | 能 Install；临时 Push 能看见标题和 tick |
| 3 | `StartUp` 改成 C | Logo 约 2 秒后进表盘 |
| 4 | `Dialplate` 改成 C（可同时加 `account_c` 薄封装） | 表盘数字、录轨按键、进地图/系统信息 |
| 5 | `SystemInfos` 改成 C | 菜单进出、各组信息刷新 |
| 6 | `StatusBar` 去掉 `namespace` | 顶栏时间/电池仍在 |
| 7 | `LiveMap`（最后；Model 可暂留 C++） | 缩放、轨迹、返回表盘 |

`PageManager` 里的页面池 / 栈 / 切页动画（`std::vector`、`std::stack`）**不要放在步骤 1 一次翻完**。步骤 1 只换生命周期调用方式。池和栈可等全部页面挂上后再单独改成 C。  
`DataCenter` / `Account` 同样后期再转；步骤 4 用 `account_c.h` 过渡即可。

`App.cpp` 的 `Install("Dialplate", "Pages/Dialplate")` **两个字符串全程不用改**。

---

## 步骤 1：虚函数换成 PageOps（先做这一步）

这一步不是改表盘，也不是一次把 `PageManager` 翻成 C。只做一件事：让调度器不再依赖 C++ 继承。

### 1.1 为什么必须先做

C 文件不能写 `class Xxx : public PageBase`。不先换成函数指针表，后面任何 `.c` 页面都挂不上去。  
整份调度器还有 vector / stack / 动画，一次改完很难验收。

### 1.2 改哪些文件

| 文件 | 动作 |
|------|------|
| `Utils/PageManager/PageBase.h` | 改成 C 能 include 的 `struct PageBase` + `PageOps`；保留原来的 `_root` / `_Manager` / `priv` 等字段。**不要叫 `struct Page`**：会和现有 `namespace Page` 撞名，编不过。 |
| `PM_Base.cpp` | `base->onCustomAttrConfig()` → `PAGE_CALL(base, on_custom_attr)` |
| `PM_State.cpp` | `onViewLoad` 等全部改成 `PAGE_CALL` |
| 现有五个页面的 `.h/.cpp` | 去掉 `public PageBase`；结构体 **第一项** 改成 `Page base`；构造时填 ops |

**不要改：** Dialplate / LiveMap 的业务逻辑、DataCenter、`App.cpp` 的 Install 字符串、切页动画实现。

### 1.3 Page 与 PageOps

```c
typedef struct PageBase PageBase;

typedef struct {
    void (*on_custom_attr)(Page *p);
    void (*on_load)(Page *p);
    void (*on_did_load)(Page *p);
    void (*on_will_appear)(Page *p);
    void (*on_did_appear)(Page *p);
    void (*on_will_disappear)(Page *p);
    void (*on_did_disappear)(Page *p);
    void (*on_unload)(Page *p);
    void (*on_did_unload)(Page *p);
} PageOps;

struct PageBase {
    const char     *name;
    lv_obj_t       *root;
    PageManager    *manager;
    const PageOps  *ops;
    /* 原 priv：state、cache、stash、anim ... */
};

#define PAGE_CALL(p, fn) do { \
    if ((p) && (p)->ops && (p)->ops->fn) (p)->ops->fn(p); \
} while (0)
```

`PAGE_STASH_POP` 现在用了 `this`，改成接收 `PageBase *` 的函数，例如 `page_stash_pop(page, &data, sizeof(data))`。

结构体必须仍叫 `PageBase`，不能叫 `Page`：页面都在 `namespace Page` 里，`typedef struct Page Page` 会让 `namespace Page` 编不过。

头文件要能被 C 和 C++ 一起用：

```c
#ifdef __cplusplus
extern "C" {
#endif
/* Page / PageOps 声明 */
#ifdef __cplusplus
}
#endif
```

### 1.4 现有 C++ 页面怎么填表（机械翻译，不改逻辑）

```c
static void on_load(Page *page)
{
    Dialplate *d = (Dialplate *)page;  /* base 在第一项，这样转合法 */
    d->onViewLoad();                   /* 继续调现有 C++ 实现 */
}

static const PageOps s_dialplate_ops = {
    .on_custom_attr    = on_custom_attr,
    .on_load           = on_load,
    .on_will_appear    = on_will_appear,
    .on_did_appear     = on_did_appear,
    .on_will_disappear = on_will_disappear,
    .on_unload         = on_unload,
    /* 空着的回调填 NULL */
};
```

构造函数里：`base.ops = &s_dialplate_ops;`。  
`new Page::Dialplate` 暂时保留。工厂宏这一步可以先不动。

五个页面都要做同一件事：`Template`、`Startup`、`Dialplate`、`SystemInfos`、`LiveMap`。漏一页，Install 或进页会空指针。

### 1.5 验收

```bash
./gengtao_build.sh run
```

开机 Logo → 表盘 → 点菜单进系统信息 → 返回。行为必须与改前一致。  
`_Template` 产品路径不会 Push，这一步没改成 `.c` 也不挡开机。

通过后再做步骤 2。

---

## 步骤 2：`_Template` 改成第一份 .c

把模板页当作菜谱，后面每一页都按同样拆法。

| 文件 | 处理 |
|------|------|
| Template.h / Template.c | `TemplatePage { Page base; view; model; timer }` + ops 表 + `Template_Create` |
| TemplateView | `struct { lv_obj_t *labelTitle, *labelTick; }` + `Create` |
| TemplateModel | `TickSave` + `GetData()` → `lv_tick_get()` |
| stash | `page_stash_pop(page, &param, sizeof(param))` |

每页只有一个实例，用静态变量：

```c
static TemplatePage s_template;

Page *Template_Create(void)
{
    memset(&s_template, 0, sizeof(s_template));
    s_template.base.ops = &s_template_ops;
    return &s_template.base;
}
```

工厂把 `APP_CLASS_MATCH(Template)` 换成表里的一行 `{ "Template", Template_Create }`。其它页仍可以 `new`。

产品路径不 Push 本页。验收：工程能编过、开机仍进表盘。若要看见本页，可临时在 `App.cpp` 里 `Push("Pages/_Template")`，看完删掉。

---

## 步骤 3：StartUp

短，适合第二份真页面。

- Model：`PlayMusic` / `SetStatusBarAppear` → Notify；`SetEncoderEnable` 仍调 HAL
- View：Logo + timeline
- Presenter：2s 一次性 timer 里 `page_replace(mgr, "Pages/Dialplate")`；关缓存、无动画
- `onTimer` 的 `user_data` 用 `StartupPage *`

验收：Logo 约 2 秒后进表盘。工厂加上 `{ "Startup", Startup_Create }`，删掉 `new Page::Startup`。

---

## 步骤 4：Dialplate（主样板）

照搬现有职责，禁止把 GPS 读进 View。对照文档 10 / 11 / 12。

- View：`ui` 嵌套结构体原样变成 C struct（源码 `lableValue` 拼写可先保留）
- Model：Init / Deinit / GetGPSReady / RecorderCommand / PlayMusic / SetStatusBarStyle / onEvent
- Presenter：`rec_state` 状态机、`Update` 刷五个 Label、`Push` 地图和系统信息

`Account` 仍是 C++。这一步写薄封装 `account_c.h`（实现仍是 `.cpp`）：

```c
typedef struct Account Account;  /* 不透明指针 */
Account *Account_Create(const char *id, void *center, uint32_t buf, void *user);
void     Account_Destroy(Account *a);
int      Account_Subscribe(Account *a, const char *name);
int      Account_Pull(Account *a, const char *name, void *buf, uint32_t size);
int      Account_Notify(Account *a, const char *name, const void *buf, uint32_t size);
void     Account_SetCallback(Account *a, int (*cb)(Account *, int event, void *from, void *data, uint32_t size));
```

`DialplateModel.c` 只 `#include "account_c.h"`。  
`new Account(..., 0, this)` → `Account_Create(..., 0, m)`，回调里 `user` 转回 `DialplateModel *`。

不要在这一步把整个 `Utils/DataCenter` 转成 C。

验收：开机 → 表盘数字刷新 → 录轨按键状态变化 → 菜单进系统信息并返回。地图入口能 Push 即可（地图页本身仍是 C++ 也可以）。

---

## 步骤 5：SystemInfos

结构重复：八组 `item_t`。Presenter 的 `Update` 里一串 `Model.GetXxx` + `View.SetXxx` 逐段搬。  
`sizeof(View.ui)/sizeof(item_t)` 这种把 ui 当数组用的写法，转 C 后只要布局仍是八个连续 `item_t` 就可以保留。

验收：表盘进系统信息，各组能刷，能返回表盘。

---

## 步骤 6：StatusBar

本来就没有 View/Model class，只有 `StatusBar_Create` + 文件内 static UI。  
去掉 `namespace Page`，`App.cpp` 改为调用 `StatusBar_Create(lv_layer_top())`。  
`DATA_PROC_INIT_DEF(StatusBar)` 仍可留在 `.cpp`，直到 DataProc 也转 C。

验收：顶栏时间、电池仍显示。

---

## 步骤 7：LiveMap（最后）

| C++ | 处理 |
|-----|------|
| `LiveMap.cpp` 里 lambda 定时器 | 改 `static void livemap_on_timer(lv_timer_t *)` |
| `pointFilter.SetOutputPointCallback([](...){...})` | 文件内 static 回调，`userData = d` |
| `Model.mapConv` / `tileConv` / `vector` | 工具库暂留 C++：**Model 可暂时保留 `.cpp` + `extern "C"` 头** |
| `static uint16_t mapLevelCurrent` | 文件内 `static` 全局，语义不变 |
| `priv` 与 Page 的 `priv` 重名 | 改成 `LiveMapPriv run;` |

可接受的过渡：`LiveMapView.c` + `LiveMap.c` 为 C，`LiveMapModel.cpp` 仍 C++。等 MapConv 转 C 再把 Model 也改掉。

验收：表盘进地图、缩放、轨迹、返回表盘。

---

## 步骤做完之后（可选，不要提前做）

- 把 `PageManager` 的页面池、导航栈、切页动画从 `std::vector` / `std::stack` 改成 C 数组
- 工厂完全改成一张表，去掉全部 `APP_CLASS_MATCH` / `new`
- 再把 `Utils/DataCenter` 转成 C（思想见文档 17 / 09）

---

## 语法对照表（每一步都用）

| 现在 C++ | 改成 C |
|----------|--------|
| `namespace Page { }` | 删掉。函数加前缀：`Dialplate_OnViewLoad` |
| `class Dialplate : public PageBase` | `struct DialplatePage { Page base; ... }`，**`base` 必须是第一项** |
| `virtual void onViewLoad()` | `PageOps` 里的函数指针，静态函数填表 |
| `this` | 第一个参数 `DialplatePage *d` 或 `Page *page` |
| `static void onEvent(...)` | 本来就是 C 函数，保留；`user_data` 仍传页面指针 |
| `new Page::Dialplate` | 步骤 1 仍可 `new`；改成 `.c` 后用静态单例 + `Xxx_Create` |
| `delete` | `free`；静态单例则只 Deinit |
| `View.Create(_root)` | `DialplateView_Create(&d->view, page->root)` |
| `Model.Init()` | `DialplateModel_Init(&d->model)` |
| `_Manager->Push("Pages/LiveMap")` | `page_push(page->manager, "Pages/LiveMap", NULL)` |
| `_Name` | `page->name` |
| `nullptr` | `NULL` |
| 引用 `TileConv::Point_t` | 先保留工具库为 C++，用 `extern "C"` 包一层 |
| lambda `[](lv_timer_t *t){...}` | 文件内 `static` 函数 |

成员函数调用时编译器偷偷传 `this`。C 里必须自己写进参数。

---

## 单个页面的标准拆法（步骤 2 起按这个抄）

目录名可先不动：

```
Pages/Dialplate/
  Dialplate.h / Dialplate.c           /* Presenter */
  DialplateView.h / DialplateView.c
  DialplateModel.h / DialplateModel.c
```

头文件去掉 `namespace`、`class`、`virtual`、`public/private`。  
private 在 C 里靠：只在 `.c` 里声明 `static` 函数，不放进头文件。

```c
/* Dialplate.h */
typedef struct {
    Page            base;
    DialplateView   view;
    DialplateModel  model;
    lv_timer_t     *timer;
    RecordState_t   rec_state;
    lv_obj_t       *last_focus;
} DialplatePage;

Page *Dialplate_Create(void);
```

```c
static void on_load(Page *page)
{
    DialplatePage *d = (DialplatePage *)page;
    DialplateModel_Init(&d->model);
    DialplateView_Create(&d->view, page->root);
}

static const PageOps s_dialplate_ops = {
    .on_custom_attr    = on_custom_attr,
    .on_load           = on_load,
    .on_will_appear    = on_will_appear,
    .on_did_appear     = on_did_appear,
    .on_will_disappear = on_will_disappear,
    .on_unload         = on_unload,
};
```

生命周期里原来做什么，**原样搬**，只改调用形式。对照表盘：

| C++ | C |
|-----|---|
| `onViewLoad` | `DialplateModel_Init` + `DialplateView_Create` + 绑三个按钮 |
| `onViewDidAppear` | `lv_timer_create(on_timer, 1000, d)` |
| `onViewWillDisappear` | 记焦点、摘 group、`lv_timer_del` |
| `onViewUnload` | `DialplateModel_Deinit` + `DialplateView_Delete` |

LVGL 回调已经是 C：

```c
static void on_event(lv_event_t *e)
{
    DialplatePage *d = lv_event_get_user_data(e);
}

lv_obj_add_event_cb(obj, on_event, LV_EVENT_ALL, d);  /* 不要传 this */
```

工厂最终长这样（每改完一页加一行，删掉对应 `APP_CLASS_MATCH`）：

```c
typedef Page *(*PageCreateFn)(void);

static const struct {
    const char    *class_name;
    PageCreateFn   create;
} k_page_table[] = {
    { "Template",    Template_Create },
    { "Startup",     Startup_Create },
    { "Dialplate",   Dialplate_Create },
    { "SystemInfos", SystemInfos_Create },
    { "LiveMap",     LiveMap_Create },
};

Page *AppFactory_CreatePage(const char *name)
{
    size_t i;
    for (i = 0; i < sizeof(k_page_table)/sizeof(k_page_table[0]); i++) {
        if (strcmp(name, k_page_table[i].class_name) == 0)
            return k_page_table[i].create();
    }
    return NULL;
}
```

---

## 工程与编译

LinuxSDL2 的 Makefile 已经 `find USER/App -name "*.c"`，新 `.c` 会被编进去。同时删掉对应 `.cpp`。

Keil 工程要手动把 `.cpp` 换成 `.c`。

`Pages/Page.h` 现在是一堆 C++ 头。转完后拆成：

- `page.h`：`Page` / `PageOps` / 各页都要的 LVGL
- 不要让 View.h 再 include 整个 PageManager

编译运行：

```bash
./gengtao_build.sh          # 只编译
./gengtao_build.sh run      # 编译并在 LinuxSDL2 目录启动
./gengtao_build.sh clean    # 删除 .o 和 xtrack
```

---

## 每页开工前的检查单（步骤 2 起）

1. 列出该页所有 `virtual onViewXxx` 和非空实现，抄到 ops 表
2. 列出 `static` 回调（timer/event），确认 `user_data` 改成 `XxxPage *`
3. View 只留 `lv_obj_*`；Model 只留 Account 封装
4. 所有 `_Manager->Push/Pop/Replace` 换成 C 的 `page_*`
5. `new` / `delete` 清掉
6. 编过、跑过该页主路径再动下一页

---

## 对照阅读（改哪看哪）

| 改这个 | 先看 |
|--------|------|
| 步骤 1 生命周期 ops | `PageBase.h`、`PM_State.cpp`、`PM_Base.cpp`、文档 16 |
| 工厂表 | `AppFactory.cpp` |
| 表盘三文件 | 文档 10 / 11 / 12 |
| 总线账号 | `DialplateModel.cpp`、文档 17 |
| 六个文件夹业务别搞混 | 文档 15 |

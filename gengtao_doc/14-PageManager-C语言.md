# 14 PageManager（C 语言）

PageManager 是本仓库的 **页面调度器**。它不管控件怎么画、GPS 怎么读，只做三件事：

1. 按名字找到页面对象（页面池）
2. 用栈记住「现在在哪、返回去哪」
3. 按状态机调用 `onViewLoad` / `onViewDidAppear` 等回调，并配上切页动画

MVP 怎么拆文件，见 [06-页面框架-C语言说明.md](06-页面框架-C语言说明.md) 和 [12-Dialplate-Presenter-C语言.md](12-Dialplate-Presenter-C语言.md)。  
本文只讲 **调度器本身**，对照 `USER/App/Utils/PageManager/`。

---

## 1. 源码文件怎么分工

| 文件 | 职责（用 C 怎么说） |
|------|---------------------|
| `PageBase.h` / `PageBase.cpp` | 一页的公共头：名字、root、状态、缓存、stash；以及各生命周期回调 |
| `PageFactory.h` | 工厂接口：`CreatePage(name)`，按字符串 new 一页 |
| `PageManager.h` | 调度器对外 API：Install / Push / Pop / Replace / BackHome |
| `PM_Base.cpp` | 页面池、页面栈、Install / Uninstall / Register |
| `PM_Router.cpp` | 路由：Push / Pop / Replace / SwitchTo / 动画结束回调 |
| `PM_State.cpp` | 生命周期状态机，真正去调 `onViewXxx` |
| `PM_Anim.cpp` | 切页动画参数表（从哪滑到哪） |
| `PM_Drag.cpp` | 根节点拖拽，松手后可能 Pop |
| `AppFactory.cpp` | 本工程的工厂实现：名字 → `new Page::Dialplate` 等 |
| `App.cpp` | 创建 manager、Install 五页、`Push("Pages/Startup")` |

读的顺序建议：`App.cpp` → `AppFactory.cpp` → `PageManager.h` → `PM_State.cpp` → `PM_Router.cpp`。

---

## 2. 先记住两个容器：池 vs 栈

这是读源码最容易搞反的一点。

```
页面池 PagePool     已经 Install 的全部页面对象（本工程 5 个）
页面栈 PageStack     导航历史，栈顶 = 当前正在显示的页
```

用 C 写就是：

```c
#define PAGE_POOL_MAX  8
#define PAGE_STACK_MAX 8

typedef struct {
    Page         *pool[PAGE_POOL_MAX];   /* 装过的页，对象一直活着 */
    int           pool_n;

    Page         *stack[PAGE_STACK_MAX]; /* 导航栈 */
    int           top;                   /* -1 表示空 */

    Page         *current;
    Page         *prev;

    PageFactory  *factory;               /* CreatePage 函数 */
    /* 动画忙标记、全局动画类型…… */
} PageManager;
```

**Install 时就已经 `new` 出页面对象，放进池里。Push 不会再 new。**  
Push 只是：按名字在池里找到那个指针，压进栈，然后跑状态机。

所以：

- 开机后五页的 C++ 对象都在 RAM 里，即使还没显示
- 同一页不能 Push 两次（栈里已经有就会失败）
- Uninstall 才会 `delete` 对象；平时切走只是卸 UI（`lv_obj`），对象还在池里

文档 06 里用「Push 时 create」是为了好懂；对照源码时，create 发生在 Install。

---

## 3. 一页长什么样（PageBase）

C++ 的 `class PageBase` 对 C 就是「公共头 + 操作表」：

```c
typedef enum {
    PAGE_STATE_IDLE,
    PAGE_STATE_LOAD,
    PAGE_STATE_WILL_APPEAR,
    PAGE_STATE_DID_APPEAR,
    PAGE_STATE_ACTIVITY,        /* 前台，用户正在看 */
    PAGE_STATE_WILL_DISAPPEAR,
    PAGE_STATE_DID_DISAPPEAR,
    PAGE_STATE_UNLOAD
} PageState;

typedef struct {
    void *ptr;
    uint32_t size;
} PageStash;                    /* 切页时带过去的一块参数 */

typedef struct Page Page;
typedef struct {
    void (*on_custom_attr)(Page *p);   /* Install 后立刻调，配缓存/动画 */
    void (*on_load)(Page *p);
    void (*on_did_load)(Page *p);
    void (*on_will_appear)(Page *p);
    void (*on_did_appear)(Page *p);
    void (*on_will_disappear)(Page *p);
    void (*on_did_disappear)(Page *p);
    void (*on_unload)(Page *p);
    void (*on_did_unload)(Page *p);
} PageOps;

struct Page {
    const char   *name;          /* 应用名，如 "Pages/Dialplate" */
    lv_obj_t     *root;          /* 这一页的 LVGL 根对象，未加载时为 NULL */
    PageManager  *manager;
    const PageOps *ops;
    PageState     state;

    int           cached;        /* 切走后是否保留 root */
    int           auto_cache;    /* 是否让调度器自动决定 cached */
    PageStash     stash;
};
```

`virtual onViewLoad()` 就是 `ops->on_load`。调度器从不写 `if (name == Dialplate)`，只调函数指针。

---

## 4. 生命周期状态机（核心）

实现：`PM_State.cpp` 的 `StateUpdate()`。  
每次切页会对 **旧页** 和 **新页** 各跑一遍。

```
IDLE
  -> LOAD              创建 root，调 onViewLoad / onViewDidLoad
  -> WILL_APPEAR       调 onViewWillAppear，开始切页动画
  -> DID_APPEAR        动画结束，调 onViewDidAppear
  -> ACTIVITY          停在前台

离开时：

  ACTIVITY
  -> WILL_DISAPPEAR    调 onViewWillDisappear，开始离开动画
  -> DID_DISAPPEAR     隐藏 root，调 onViewDidDisappear
       |-- 有缓存 --> WILL_APPEAR（root 还在，只是藏着，下次直接出现）
       `-- 无缓存 --> UNLOAD --> onViewUnload / DidUnload --> IDLE
                                   （异步删 root，stash 也释放）
```

几个实现细节：

| 点 | 源码行为 |
|----|----------|
| LOAD 时创建 root | `lv_obj_create(lv_scr_act())`，铺上 `SetRootDefaultStyle` 的全屏黑底 |
| root 的 user_data | 指向这一页的 `PageBase*`，拖拽事件能找回页面 |
| 进入 ACTIVITY | `onViewDidAppear` 之后才算前台，适合在这里开 1s timer |
| 有缓存的离开 | `DID_DISAPPEAR` 后状态变成 `WILL_APPEAR`，**不再 UNLOAD** |
| UNLOAD | `lv_obj_del_async(root)`，避免动画还没完就删对象 |

表盘把资源钉在这些回调上（详见文档 12）：

| 回调 | 表盘做什么 |
|------|------------|
| `onViewLoad` | Model.Init、View.Create、绑按键 |
| `onViewWillAppear` | 编码器 group、刷一帧、出现动画 |
| `onViewDidAppear` | 开 1s 刷新 timer |
| `onViewWillDisappear` | 记住焦点、摘 group、删 timer |
| `onViewUnload` | Model.Deinit、View.Delete |

---

## 5. Install：按类名创建，按应用名登记

`App.cpp`：

```c
manager.Install("Dialplate", "Pages/Dialplate");
```

第一个参数是 **工厂里的类名**，第二个是 **以后 Push 用的名字**。

`PM_Base.cpp` 里 Install 做的事，用 C 写：

```c
int page_install(PageManager *pm, const char *class_name, const char *app_name)
{
    if (find_in_pool(pm, app_name))
        return 0;                          /* 不许重名 */

    Page *p = pm->factory->create(class_name);  /* AppFactory::CreatePage */
    if (!p)
        return 0;

    memset(&p->priv, 0, sizeof(p->priv));
    p->root = NULL;
    p->name = app_name;
    p->manager = pm;
    pm->pool[pm->pool_n++] = p;

    if (p->ops->on_custom_attr)
        p->ops->on_custom_attr(p);         /* 配缓存、动画 */

    return 1;
}
```

工厂 `AppFactory.cpp` 就是一张名字表：

```c
Page *CreatePage(const char *name)
{
    if (strcmp(name, "Dialplate") == 0) return Dialplate_Create();
    if (strcmp(name, "LiveMap")   == 0) return LiveMap_Create();
    if (strcmp(name, "Startup")   == 0) return Startup_Create();
    /* Template / SystemInfos 同理 */
    return NULL;
}
```

本工程安装的五页：

| className | appName（Push 用这个） |
|-----------|------------------------|
| Template | `Pages/_Template` |
| LiveMap | `Pages/LiveMap` |
| Dialplate | `Pages/Dialplate` |
| SystemInfos | `Pages/SystemInfos` |
| Startup | `Pages/Startup` |

状态栏 `StatusBar` **不是页面**，画在 `lv_layer_top()`，不进池、不进栈。

`onCustomAttrConfig` 在 Install 时就执行，页面还没 LOAD。适合设缓存和默认动画，例如：

- Startup / LiveMap：`SetCustomCacheEnable(false)`，切走就卸 UI（地图控件很重）
- Template：强制缓存 + 自定义底部滑入动画
- Dialplate：`LOAD_ANIM_NONE`，切到表盘不要滑场

---

## 6. 路由 API：Push / Pop / Replace / BackHome

实现：`PM_Router.cpp`。四个接口都先检查「动画是不是还在忙」，忙就直接失败，避免连点把栈弄乱。

### Push：压栈进入新页

```c
int page_push(PageManager *pm, const char *name, const PageStash *stash)
{
    if (anim_busy(pm)) return 0;
    if (find_in_stack(pm, name)) return 0;     /* 禁止同一页在栈里出现两次 */
    Page *next = find_in_pool(pm, name);
    if (!next) return 0;

    pm->stack[++pm->top] = next;
    return switch_to(pm, next, /*is_enter=*/1, stash);
}
```

旧页还在栈里，只是进入消失流程。  
默认自动缓存时，旧页切走后 **root 隐藏但还在**，返回时不用再 `onViewLoad`。

本工程：

- 开机：`Push("Pages/Startup")`
- 表盘按地图：`Push("Pages/LiveMap")`
- 表盘按菜单：`Push("Pages/SystemInfos")`

### Pop：弹出当前页，回到栈里下一页

```c
int page_pop(PageManager *pm)
{
    if (anim_busy(pm)) return 0;
    if (pm->top <= 0) return 0;                /* 只剩根页，不许再弹 */

    Page *leaving = pm->stack[pm->top--];
    if (leaving->auto_cache)
        leaving->cached = 0;                   /* 被弹出的页默认卸掉 UI */

    return switch_to(pm, pm->stack[pm->top], /*is_enter=*/0, NULL);
}
```

注意：Pop 关掉的是 **被弹出那一页** 的缓存。底下那页如果当初 Push 时缓存了，会走 `WILL_APPEAR` 直接显示。

地图 / 系统信息页里返回，都是 `_Manager->Pop()`，它们不用知道底下是表盘。

### Replace：换成另一页，旧页出栈

Startup 2 秒后：

```c
_Manager->Replace("Pages/Dialplate");
```

源码会：把栈顶 `IsCached = false`（强制卸），pop 掉，再 push 新页。  
开机动画不必压在栈底占一份缓存，结束后栈里只剩表盘，这就是「根页」。

### BackHome：清到栈底那一页

`SetStackClear(true)` 强制卸掉中间各页，只留最底下，再 SwitchTo 过去。  
本工程页面里几乎不用，API 留着给「连按返回直到主界面」。

### 对照本工程栈变化

```
Push Startup
    栈: [ Startup ]

Replace Dialplate
    栈: [ Dialplate ]          Startup UNLOAD

Push LiveMap
    栈: [ Dialplate, LiveMap ] Dialplate 隐藏并缓存

Pop
    栈: [ Dialplate ]          LiveMap UNLOAD（它关了缓存）
                               Dialplate 直接 WILL_APPEAR
```

---

## 7. SwitchTo：一次切换实际跑什么

`Push` / `Pop` / `Replace` 最后都进 `SwitchTo`。

顺序可以记成：

1. 若带了 stash，把参数 `memcpy` 到目标页（没有就 `lv_mem_alloc`）
2. `current = 新页`
3. 新页若已缓存 → 状态设为 `WILL_APPEAR`（跳过 LOAD）  
   否则 → 状态设为 `LOAD`
4. 旧页标记为「离开方」，新页标记为「进入方」
5. `StateUpdate(旧页)`，再 `StateUpdate(新页)`
6. 按进入/退出调整两个 root 的图层（谁在上面）
7. 动画在 `WILL_APPEAR` / `WILL_DISAPPEAR` 里 `lv_anim_start`
8. 动画结束回调 `onSwitchAnimFinish` 再 `StateUpdate` 一次，进入 DID_APPEAR / DID_DISAPPEAR
9. 双方动画都结束，才把 `IsSwitchReq` 清掉，允许下一次 Push

所以切页不是同步函数一口气跑完生命周期，而是：

```
本次调用：LOAD + WILL_APPEAR（启动动画）
过几百毫秒：动画 ready_cb → DID_APPEAR → ACTIVITY
```

`LOAD_ANIM_NONE` 时动画时长按实现仍会走 anim 接口，但位移为 0，表盘就是这种。

---

## 8. 缓存：切走后 root 还在不在

LOAD 结束时决定 `IsCached`：

```c
if (page->disable_auto_cache)
    page->cached = page->req_enable_cache;   /* 页面自己说了算 */
else
    page->cached = 1;                        /* 默认缓存 */
```

| 页面 | 配置 | 效果 |
|------|------|------|
| Dialplate（默认） | 自动缓存 | Push 到地图后，表盘控件还在，返回很快 |
| Startup | `SetCustomCacheEnable(false)` | Replace 后立刻 UNLOAD |
| LiveMap | 同上 | Pop 后卸掉瓦片/轨迹，省 RAM |
| Template | `SetCustomCacheEnable(true)` | 演示手动开缓存 |

`SetCustomCacheEnable` 会同时关掉自动缓存，改由页面自己指定。

---

## 9. Stash：切页带参数

类似「打开这一页时附带一份数据」。  
Push 时可传 `Stash_t { ptr, size }`，宏：

```c
#define PAGE_STASH_MAKE(data)  { &(data), sizeof(data) }
#define PAGE_STASH_POP(data)   StashPop(&(data), sizeof(data))
```

SwitchTo 里会拷到目标页的 `priv.Stash`；目标页在 `onViewWillAppear` 里 `PAGE_STASH_POP` 取走。  
本工程正式页面没用，模板 `Pages/_Template/Template.cpp` 示范了取颜色和 timer 周期。

没有 stash 时 `PAGE_STASH_POP` 失败，模板里先填了默认值再 pop，失败就用默认。

UNLOAD 时若 stash 还在，会 `lv_mem_free`。

---

## 10. 切页动画和拖拽

全局默认在 `App.cpp`：

```c
manager.SetGlobalLoadAnimType(LOAD_ANIM_OVER_TOP);
```

单页可在 `onCustomAttrConfig` 里 `SetCustomLoadAnimType` 覆盖。类型（`PageManager.h`）：

| 类型 | 含义 |
|------|------|
| `OVER_*` | 新页盖住旧页，旧页不动 |
| `MOVE_*` | 新页把旧页推走 |
| `FADE_ON` | 淡入淡出 |
| `NONE` | 无动画 |
| `GLOBAL` | 跟随全局设置 |

`PM_Anim.cpp` 按屏幕宽高填 start/end。  
OVER 且方向不是 NONE 时，LOAD 会给新页 root 开拖拽（`PM_Drag.cpp`）：手指拖当前页，露出底下缓存页，松手超过阈值就异步 Pop。

动画进行中 Push/Pop 会被丢掉（`SwitchAnimStateCheck`）。按钮回调里连续切页，第二次可能失败，这是保护不是 bug。

---

## 11. 和页面代码怎么接上

页面里跳转，走的是基类上的 `_Manager`：

```c
/* Dialplate.cpp */
_Manager->Push("Pages/LiveMap");
_Manager->Push("Pages/SystemInfos");

/* LiveMap / SystemInfos */
_Manager->Pop();

/* StartUp.cpp，2s 定时器 */
_Manager->Replace("Pages/Dialplate");
```

C 等价：`page->manager` 指向那个全局 `PageManager`。  
不要在页面里自己 `lv_scr_load` 换屏，否则状态机和栈会和真实显示对不上。

新增一页的固定步骤（和文档 04 相同）：

1. 复制 `Pages/_Template`
2. `AppFactory.cpp` 加 `APP_CLASS_MATCH`
3. `App.cpp` 里 `Install("类名", "Pages/名字")`
4. 从已有页 `Push("Pages/名字")`

---

## 12. 用 C 手写时最小骨架

如果要从这套调度器学「自己做多页面」，不必抄动画和拖拽，先有池 + 栈 + 状态机就够：

```c
void switch_to(PageManager *pm, Page *next, int is_enter)
{
    Page *old = pm->current;
    pm->prev = old;
    pm->current = next;

    if (old) {
        old->state = PAGE_STATE_ACTIVITY;  /* 触发离开 */
        state_update(old);
    }

    next->state = next->cached ? PAGE_STATE_WILL_APPEAR
                               : PAGE_STATE_LOAD;
    state_update(next);
}

void state_update(Page *p)
{
    switch (p->state) {
    case PAGE_STATE_LOAD:
        p->root = lv_obj_create(lv_scr_act());
        if (p->ops->on_load) p->ops->on_load(p);
        p->state = PAGE_STATE_WILL_APPEAR;
        state_update(p);
        break;
    case PAGE_STATE_WILL_APPEAR:
        if (p->ops->on_will_appear) p->ops->on_will_appear(p);
        /* 这里可 lv_anim_start，结束再进 DID_APPEAR */
        p->state = PAGE_STATE_DID_APPEAR;
        if (p->ops->on_did_appear) p->ops->on_did_appear(p);
        p->state = PAGE_STATE_ACTIVITY;
        break;
    case PAGE_STATE_ACTIVITY:
        p->state = PAGE_STATE_WILL_DISAPPEAR;
        if (p->ops->on_will_disappear) p->ops->on_will_disappear(p);
        if (p->ops->on_did_disappear) p->ops->on_did_disappear(p);
        if (p->cached) {
            lv_obj_add_flag(p->root, LV_OBJ_FLAG_HIDDEN);
            p->state = PAGE_STATE_WILL_APPEAR;
        } else {
            p->state = PAGE_STATE_UNLOAD;
            state_update(p);
        }
        break;
    case PAGE_STATE_UNLOAD:
        if (p->ops->on_unload) p->ops->on_unload(p);
        lv_obj_del(p->root);
        p->root = NULL;
        p->state = PAGE_STATE_IDLE;
        break;
    default:
        break;
    }
}
```

源码比这多了：动画 busy、图层前后、stash、拖拽、Replace 强制卸缓存。  
自己做菜单树时，先保证 **切走必停 timer、必摘 indev group**，再考虑动画。

---

## 13. 对照阅读清单

| 想搞清 | 打开 |
|--------|------|
| 对外 API、动画枚举 | `Utils/PageManager/PageManager.h` |
| 生命周期回调清单 | `Utils/PageManager/PageBase.h` |
| 状态怎么跳 | `PM_State.cpp` |
| Push/Pop/Replace | `PM_Router.cpp` |
| 池和栈 | `PM_Base.cpp` |
| 开机安装 | `USER/App/App.cpp` |
| 名字 → new | `Pages/AppFactory.cpp` |
| Replace 实例 | `Pages/StartUp/StartUp.cpp` |
| Push/Pop 实例 | `Pages/Dialplate/Dialplate.cpp` |
| 一页如何填回调 | 文档 12，以及 `Pages/_Template/` |

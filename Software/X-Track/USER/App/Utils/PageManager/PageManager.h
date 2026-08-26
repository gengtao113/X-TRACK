#ifndef __PAGE_MANAGER_H
#define __PAGE_MANAGER_H

#include "PageBase.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PAGE_POOL_MAX
#define PAGE_POOL_MAX  8
#endif
#ifndef PAGE_STACK_MAX
#define PAGE_STACK_MAX 8
#endif

typedef PageBase* (*PageCreateFn)(const char* name);

typedef enum
{
    LOAD_ANIM_GLOBAL = 0,
    LOAD_ANIM_OVER_LEFT,
    LOAD_ANIM_OVER_RIGHT,
    LOAD_ANIM_OVER_TOP,
    LOAD_ANIM_OVER_BOTTOM,
    LOAD_ANIM_MOVE_LEFT,
    LOAD_ANIM_MOVE_RIGHT,
    LOAD_ANIM_MOVE_TOP,
    LOAD_ANIM_MOVE_BOTTOM,
    LOAD_ANIM_FADE_ON,
    LOAD_ANIM_NONE,
    _LOAD_ANIM_LAST = LOAD_ANIM_NONE
} LoadAnim_t;

typedef enum
{
    ROOT_DRAG_DIR_NONE,
    ROOT_DRAG_DIR_HOR,
    ROOT_DRAG_DIR_VER
} RootDragDir_t;

typedef void (*lv_anim_setter_t)(void*, int32_t);
typedef int32_t (*lv_anim_getter_t)(void*);

typedef struct
{
    struct
    {
        int32_t start;
        int32_t end;
    } enter;
    struct
    {
        int32_t start;
        int32_t end;
    } exit;
} AnimValue_t;

typedef struct
{
    lv_anim_setter_t setter;
    lv_anim_getter_t getter;
    RootDragDir_t dragDir;
    AnimValue_t push;
    AnimValue_t pop;
} LoadAnimAttr_t;

struct PageManager
{
    PageCreateFn _CreatePage;
    PageBase* _PagePool[PAGE_POOL_MAX];
    int _PagePoolN;
    PageBase* _PageStack[PAGE_STACK_MAX];
    int _PageStackN;
    PageBase* _PagePrev;
    PageBase* _PageCurrent;
    struct
    {
        bool IsSwitchReq;
        bool IsBusy;
        bool IsEntering;
        PageAnimAttr_t Current;
        PageAnimAttr_t Global;
    } _AnimState;
    lv_style_t* _RootDefaultStyle;
};

void        PageManager_Init(PageManager* pm, PageCreateFn create);
void        PageManager_Deinit(PageManager* pm);
bool        PageManager_Install(PageManager* pm, const char* className, const char* appName);
bool        PageManager_Uninstall(PageManager* pm, const char* appName);
bool        PageManager_Register(PageManager* pm, PageBase* base, const char* name);
bool        PageManager_Unregister(PageManager* pm, const char* name);
bool        PageManager_Replace(PageManager* pm, const char* name, const PageStash_t* stash);
bool        PageManager_Push(PageManager* pm, const char* name, const PageStash_t* stash);
bool        PageManager_Pop(PageManager* pm);
bool        PageManager_BackHome(PageManager* pm);
const char* PageManager_GetPagePrevName(PageManager* pm);
void        PageManager_SetGlobalLoadAnimType(PageManager* pm, LoadAnim_t anim, uint16_t time, lv_anim_path_cb_t path);

static inline void PageManager_SetRootDefaultStyle(PageManager* pm, lv_style_t* style)
{
    pm->_RootDefaultStyle = style;
}

static inline bool PageManager_GetIsOverAnim(uint8_t anim)
{
    return (anim >= LOAD_ANIM_OVER_LEFT && anim <= LOAD_ANIM_OVER_BOTTOM);
}

static inline bool PageManager_GetIsMoveAnim(uint8_t anim)
{
    return (anim >= LOAD_ANIM_MOVE_LEFT && anim <= LOAD_ANIM_MOVE_BOTTOM);
}

static inline LoadAnim_t PageManager_GetCurrentLoadAnimType(const PageManager* pm)
{
    return (LoadAnim_t)pm->_AnimState.Current.Type;
}

bool PageManager_GetLoadAnimAttr(uint8_t anim, LoadAnimAttr_t* attr);

static inline bool PageManager_GetCurrentLoadAnimAttr(PageManager* pm, LoadAnimAttr_t* attr)
{
    return PageManager_GetLoadAnimAttr((uint8_t)PageManager_GetCurrentLoadAnimType(pm), attr);
}

/* internal, used across PM_*.c */
PageBase*   PageManager_FindPageInPool(PageManager* pm, const char* name);
PageBase*   PageManager_FindPageInStack(PageManager* pm, const char* name);
PageBase*   PageManager_GetStackTop(PageManager* pm);
PageBase*   PageManager_GetStackTopAfter(PageManager* pm);
void        PageManager_SetStackClear(PageManager* pm, bool keepBottom);
bool        PageManager_FourceUnload(PageManager* pm, PageBase* base);
bool        PageManager_SwitchTo(PageManager* pm, PageBase* base, bool isEnterAct, const PageStash_t* stash);
bool        PageManager_SwitchAnimStateCheck(PageManager* pm);
bool        PageManager_SwitchReqCheck(PageManager* pm);
void        PageManager_SwitchAnimCreate(PageManager* pm, PageBase* base);
void        PageManager_SwitchAnimTypeUpdate(PageManager* pm, PageBase* base);
void        PageManager_AnimDefaultInit(PageManager* pm, lv_anim_t* a);
void        PageManager_StateUpdate(PageManager* pm, PageBase* base);
PageState_t PageManager_StateLoadExecute(PageManager* pm, PageBase* base);
PageState_t PageManager_StateWillAppearExecute(PageManager* pm, PageBase* base);
PageState_t PageManager_StateDidAppearExecute(PageManager* pm, PageBase* base);
PageState_t PageManager_StateWillDisappearExecute(PageManager* pm, PageBase* base);
PageState_t PageManager_StateDidDisappearExecute(PageManager* pm, PageBase* base);
PageState_t PageManager_StateUnloadExecute(PageManager* pm, PageBase* base);
void        PageManager_RootEnableDrag(PageManager* pm, lv_obj_t* root);
void        PageManager_onRootDragEvent(lv_event_t* event);
void        PageManager_onRootDragAnimFinish(lv_anim_t* a);
void        PageManager_onRootAsyncLeave(void* base);
void        PageManager_RootGetDragPredict(lv_coord_t* x, lv_coord_t* y);
void        PageManager_onSwitchAnimFinish(lv_anim_t* a);

#ifdef __cplusplus
}
#endif

#endif

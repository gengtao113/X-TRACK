/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "PageManager.h"
#include "PM_Log.h"
#include <string.h>

 /**
   * @brief  Enter a new page, replace the old page
   * @param  name: The name of the page to enter
   * @param  stash: Parameters passed to the new page
   * @retval Return true if successful
   */
bool PageManager_Replace(PageManager* pm, const char* name, const PageStash_t* stash)
{
    /* Check whether the animation of switching pages is being executed */
    if (!PageManager_SwitchAnimStateCheck(pm))
    {
        return false;
    }

    /* Check whether the stack is repeatedly pushed  */
    if (PageManager_FindPageInStack(pm, name) != NULL)
    {
        PM_LOG_ERROR("Page(%s) was multi push", name);
        return false;
    }

    /* Check if the page is registered in the page pool */
    PageBase* base = PageManager_FindPageInPool(pm, name);

    if (base == NULL)
    {
        PM_LOG_ERROR("Page(%s) was not install", name);
        return false;
    }

    /* Get the top page of the stack */
    PageBase* top = PageManager_GetStackTop(pm);

    if (top == NULL)
    {
        PM_LOG_ERROR("Stack top is NULL");
        return false;
    }

    /* Force disable cache */
    top->priv.IsCached = false;

    /* Synchronous automatic cache configuration */
    base->priv.IsDisableAutoCache = base->priv.ReqDisableAutoCache;

    /* Remove current page */
    pm->_PageStackN--;

    if (pm->_PageStackN >= PAGE_STACK_MAX)
    {
        PM_LOG_ERROR("Page stack is full");
        return false;
    }

    /* Push into the stack */
    pm->_PageStack[pm->_PageStackN++] = base;

    PM_LOG_INFO("Page(%s) replace Page(%s) (stash = 0x%p)", name, top->_Name, stash);

    /* Page switching execution */
    return PageManager_SwitchTo(pm, base, true, stash);
}

/**
  * @brief  Enter a new page, the old page is pushed onto the stack
  * @param  name: The name of the page to enter 
  * @param  stash: Parameters passed to the new page
  * @retval Return true if successful
  */
bool PageManager_Push(PageManager* pm, const char* name, const PageStash_t* stash)
{
    /* Check whether the animation of switching pages is being executed */
    if (!PageManager_SwitchAnimStateCheck(pm))
    {
        return false;
    }

    /* Check whether the stack is repeatedly pushed  */
    if (PageManager_FindPageInStack(pm, name) != NULL)
    {
        PM_LOG_ERROR("Page(%s) was multi push", name);
        return false;
    }

    /* Check if the page is registered in the page pool */
    PageBase* base = PageManager_FindPageInPool(pm, name);

    if (base == NULL)
    {
        PM_LOG_ERROR("Page(%s) was not install", name);
        return false;
    }

    /* Synchronous automatic cache configuration */
    base->priv.IsDisableAutoCache = base->priv.ReqDisableAutoCache;

    if (pm->_PageStackN >= PAGE_STACK_MAX)
    {
        PM_LOG_ERROR("Page stack is full");
        return false;
    }

    /* Push into the stack */
    pm->_PageStack[pm->_PageStackN++] = base;

    PM_LOG_INFO("Page(%s) push >> [Screen] (stash = 0x%p)", name, stash);

    /* Page switching execution */
    return PageManager_SwitchTo(pm, base, true, stash);
}

/**
  * @brief  Pop the current page
  * @param  None
  * @retval Return true if successful
  */
bool PageManager_Pop(PageManager* pm)
{
    /* Check whether the animation of switching pages is being executed */
    if (!PageManager_SwitchAnimStateCheck(pm))
    {
        return false;
    }
     /*Check if the page is the root page */
    if (pm->_PageStackN <= 1)
    {
        PM_LOG_WARN("Only root page remains, can't pop");
        return false;
    }
    /* Get the top page of the stack */
    PageBase* top = PageManager_GetStackTop(pm);

    if (top == NULL)
    {
        PM_LOG_WARN("Page stack is empty, can't pop");
        return false;
    }

    /* Whether to turn off automatic cache */
    if (!top->priv.IsDisableAutoCache)
    {
        PM_LOG_INFO("Page(%s) has auto cache, cache disabled", top->_Name);
        top->priv.IsCached = false;
    }

    PM_LOG_INFO("Page(%s) pop << [Screen]", top->_Name);

    /* Page popup */
    pm->_PageStackN--;

    /* Get the next page */
    top = PageManager_GetStackTop(pm);

    /* Page switching execution */
    return PageManager_SwitchTo(pm, top, false, NULL);
}

/**
  * @brief  Page switching
  * @param  newNode: Pointer to new page
  * @param  isEnterAct: Whether it is a ENTER action
  * @param  stash: Parameters passed to the new page
  * @retval Return true if successful
  */
bool PageManager_SwitchTo(PageManager* pm, PageBase* newNode, bool isEnterAct, const PageStash_t* stash)
{
    if (newNode == NULL)
    {
        PM_LOG_ERROR("newNode is NULL");
        return false;
    }

    /* Whether page switching has been requested */
    if (pm->_AnimState.IsSwitchReq)
    {
        PM_LOG_WARN("Page switch busy, reqire(%s) is ignore", newNode->_Name);
        return false;
    }

    pm->_AnimState.IsSwitchReq = true;

    /* Is there a parameter to pass */
    if (stash != NULL)
    {
        PM_LOG_INFO("stash is detect, %s >> stash(0x%p) >> %s", PageManager_GetPagePrevName(pm), stash, newNode->_Name);

        void* buffer = NULL;

        if (newNode->priv.Stash.ptr == NULL)
        {
            buffer = lv_mem_alloc(stash->size);
            if (buffer == NULL)
            {
                PM_LOG_ERROR("stash malloc failed");
            }
            else
            {
                PM_LOG_INFO("stash(0x%p) malloc[%d]", buffer, stash->size);
            }
        }
        else if(newNode->priv.Stash.size == stash->size)
        {
            buffer = newNode->priv.Stash.ptr;
            PM_LOG_INFO("stash(0x%p) is exist", buffer);
        }

        if (buffer != NULL)
        {
            memcpy(buffer, stash->ptr, stash->size);
            PM_LOG_INFO("stash memcpy[%d] 0x%p >> 0x%p", stash->size, stash->ptr, buffer);
            newNode->priv.Stash.ptr = buffer;
            newNode->priv.Stash.size = stash->size;
        }
    }

    /* Record current page */
    pm->_PageCurrent = newNode;

    /* If the current page has a cache */
    if (pm->_PageCurrent->priv.IsCached)
    {
        /* Direct display, no need to load */
        PM_LOG_INFO("Page(%s) has cached, appear driectly", pm->_PageCurrent->_Name);
        pm->_PageCurrent->priv.State = PAGE_STATE_WILL_APPEAR;
    }
    else
    {
        /* Load page */
        pm->_PageCurrent->priv.State = PAGE_STATE_LOAD;
    }

    if (pm->_PagePrev != NULL)
    {
        pm->_PagePrev->priv.Anim.IsEnter = false;
    }

    pm->_PageCurrent->priv.Anim.IsEnter = true;

    pm->_AnimState.IsEntering = isEnterAct;

    if (pm->_AnimState.IsEntering)
    {
        /* Update the animation configuration according to the current page */
        PageManager_SwitchAnimTypeUpdate(pm, pm->_PageCurrent);
    }

    /* Update the state machine of the previous page */
    PageManager_StateUpdate(pm, pm->_PagePrev);

    /* Update the state machine of the current page */
    PageManager_StateUpdate(pm, pm->_PageCurrent);

    /* Move the layer, move the new page to the front */
    if (pm->_AnimState.IsEntering)
    {
        PM_LOG_INFO("Page ENTER is detect, move Page(%s) to foreground", pm->_PageCurrent->_Name);
        if (pm->_PagePrev)lv_obj_move_foreground(pm->_PagePrev->_root);
        lv_obj_move_foreground(pm->_PageCurrent->_root);
    }
    else
    {
        PM_LOG_INFO("Page EXIT is detect, move Page(%s) to foreground", PageManager_GetPagePrevName(pm));
        lv_obj_move_foreground(pm->_PageCurrent->_root);
        if (pm->_PagePrev)lv_obj_move_foreground(pm->_PagePrev->_root);
    }
    return true;
}

/**
  * @brief  Force the end of the life cycle of the page without animation 
  * @param  base: Pointer to the page being executed
  * @retval Return true if successful
  */
bool PageManager_FourceUnload(PageManager* pm, PageBase* base)
{
    if (base == NULL)
    {
        PM_LOG_ERROR("Page is NULL, Unload failed");
        return false;
    }

    PM_LOG_INFO("Page(%s) Fource unloading...", base->_Name);

    if (base->priv.State == PAGE_STATE_ACTIVITY)
    {
        PM_LOG_INFO("Page state is ACTIVITY, Disappearing...");
        PAGE_CALL(base, on_will_disappear);
        PAGE_CALL(base, on_did_disappear);
    }

    base->priv.State = PageManager_StateUnloadExecute(pm, base);

    return true;
}

/**
  * @brief  Back to the main page (the page at the bottom of the stack) 
  * @param  None
  * @retval Return true if successful
  */
bool PageManager_BackHome(PageManager* pm)
{
    /* Check whether the animation of switching pages is being executed */
    if (!PageManager_SwitchAnimStateCheck(pm))
    {
        return false;
    }

    PageManager_SetStackClear(pm, true);

    pm->_PagePrev = NULL;

    PageBase* home = PageManager_GetStackTop(pm);

    PageManager_SwitchTo(pm, home, false, NULL);

    return true;
}

/**
  * @brief  Check if the page switching animation is being executed
  * @param  None
  * @retval Return true if it is executing
  */
bool PageManager_SwitchAnimStateCheck(PageManager* pm)
{
    if (pm->_AnimState.IsSwitchReq || pm->_AnimState.IsBusy)
    {
        PM_LOG_WARN(
            "Page switch busy[AnimState.IsSwitchReq = %d,"
            "AnimState.IsBusy = %d],"
            "request ignored",
            pm->_AnimState.IsSwitchReq,
            pm->_AnimState.IsBusy
        );
        return false;
    }

    return true;
}

/**
  * @brief  Page switching request check 
  * @param  None
  * @retval Return true if all pages are executed
  */
bool PageManager_SwitchReqCheck(PageManager* pm)
{
    bool ret = false;
    bool lastNodeBusy = pm->_PagePrev && pm->_PagePrev->priv.Anim.IsBusy;

    if (!pm->_PageCurrent->priv.Anim.IsBusy && !lastNodeBusy)
    {
        PM_LOG_INFO("----Page switch was all finished----");
        pm->_AnimState.IsSwitchReq = false;
        ret = true;
        pm->_PagePrev = pm->_PageCurrent;
    }
    else
    {
        if (pm->_PageCurrent->priv.Anim.IsBusy)
        {
            PM_LOG_WARN("Page PageCurrent(%s) is busy", pm->_PageCurrent->_Name);
        }
        else
        {
            PM_LOG_WARN("Page PagePrev(%s) is busy", PageManager_GetPagePrevName(pm));
        }
    }

    return ret;
}

/**
  * @brief  PPage switching animation execution end callback 
  * @param  a: Pointer to animation
  * @retval None
  */
void PageManager_onSwitchAnimFinish(lv_anim_t* a)
{
    PageBase* base = (PageBase*)lv_anim_get_user_data(a);
    PageManager* manager = base->_Manager;

    PM_LOG_INFO("Page(%s) Anim finish", base->_Name);

    PageManager_StateUpdate(manager, base);
    base->priv.Anim.IsBusy = false;
    bool isFinished = PageManager_SwitchReqCheck(manager);

    if (!manager->_AnimState.IsEntering && isFinished)
    {
        PageManager_SwitchAnimTypeUpdate(manager, manager->_PageCurrent);
    }
}

/**
  * @brief  Create page switching animation
  * @param  a: Point to the animated page
  * @retval None
  */
void PageManager_SwitchAnimCreate(PageManager* pm, PageBase* base)
{
    LoadAnimAttr_t animAttr;
    if (!PageManager_GetCurrentLoadAnimAttr(pm, &animAttr))
    {
        return;
    }

    lv_anim_t a;
    PageManager_AnimDefaultInit(pm, &a);
    lv_anim_set_user_data(&a, base);
    lv_anim_set_var(&a, base->_root);
    lv_anim_set_ready_cb(&a, PageManager_onSwitchAnimFinish);
    lv_anim_set_exec_cb(&a, animAttr.setter);

    int32_t start = 0;

    if (animAttr.getter)
    {
        start = animAttr.getter(base->_root);
    }

    if (pm->_AnimState.IsEntering)
    {
        if (base->priv.Anim.IsEnter)
        {
            lv_anim_set_values(
                &a,
                animAttr.push.enter.start,
                animAttr.push.enter.end
            );
        }
        else /* Exit */
        {
            lv_anim_set_values(
                &a,
                start,
                animAttr.push.exit.end
            );
        }
    }
    else /* Pop */
    {
        if (base->priv.Anim.IsEnter)
        {
            lv_anim_set_values(
                &a,
                animAttr.pop.enter.start,
                animAttr.pop.enter.end
            );
        }
        else /* Exit */
        {
            lv_anim_set_values(
                &a,
                start,
                animAttr.pop.exit.end
            );
        }
    }

    lv_anim_start(&a);
    base->priv.Anim.IsBusy = true;
}

/**
  * @brief  Set global animation properties 
  * @param  anim: Animation type
  * @param  time: Animation duration
  * @param  path: Animation curve
  * @retval None
  */
void PageManager_SetGlobalLoadAnimType(PageManager* pm, LoadAnim_t anim, uint16_t time, lv_anim_path_cb_t path)
{
    if (anim > _LOAD_ANIM_LAST)
    {
        anim = LOAD_ANIM_NONE;
    }

    pm->_AnimState.Global.Type = anim;
    pm->_AnimState.Global.Time = time;
    pm->_AnimState.Global.Path = path;

    PM_LOG_INFO("Set global load anim type = %d", anim);
}

/**
  * @brief  Update current animation properties, apply page custom animation
  * @param  base: Pointer to page
  * @retval None
  */
void PageManager_SwitchAnimTypeUpdate(PageManager* pm, PageBase* base)
{
    if (base->priv.Anim.Attr.Type == LOAD_ANIM_GLOBAL)
    {
        PM_LOG_INFO(
            "Page(%s) Anim.Type was not set, use AnimState.Global.Type = %d",
            base->_Name,
            pm->_AnimState.Global.Type
        );
        pm->_AnimState.Current = pm->_AnimState.Global;
    }
    else
    {
        if (base->priv.Anim.Attr.Type > _LOAD_ANIM_LAST)
        {
            PM_LOG_ERROR(
                "Page(%s) ERROR custom Anim.Type = %d, use AnimState.Global.Type = %d",
                base->_Name,
                base->priv.Anim.Attr.Type,
                pm->_AnimState.Global.Type
            );
            base->priv.Anim.Attr = pm->_AnimState.Global;
        }
        else
        {
            PM_LOG_INFO(
                "Page(%s) custom Anim.Type set = %d",
                base->_Name,
                base->priv.Anim.Attr.Type
            );
        }
        pm->_AnimState.Current = base->priv.Anim.Attr;
    }
}

/**
  * @brief  Set animation default parameters
  * @param  a: Pointer to animation
  * @retval None
  */
void PageManager_AnimDefaultInit(PageManager* pm, lv_anim_t* a)
{
    lv_anim_init(a);

    uint32_t time = (PageManager_GetCurrentLoadAnimType(pm) == LOAD_ANIM_NONE) ? 0 : pm->_AnimState.Current.Time;
    lv_anim_set_time(a, time);
    lv_anim_set_path_cb(a, pm->_AnimState.Current.Path);
}

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

#define PM_EMPTY_PAGE_NAME "EMPTY_PAGE"

/**
  * @brief  Page manager constructor
  * @param  create: Page constructor by class name
  * @retval None
  */
void PageManager_Init(PageManager* pm, PageCreateFn create)
{
    memset(pm, 0, sizeof(*pm));
    pm->_CreatePage = create;
    PageManager_SetGlobalLoadAnimType(pm, LOAD_ANIM_OVER_LEFT, 500, lv_anim_path_ease_out);
}

/**
  * @brief  Page manager destructor
  * @param  None
  * @retval None
  */
void PageManager_Deinit(PageManager* pm)
{
    PageManager_SetStackClear(pm, false);
}

/**
  * @brief  Search pages in the page pool
  * @param  name: Page name
  * @retval A pointer to the base class of the page, or NULL if not found
  */
PageBase* PageManager_FindPageInPool(PageManager* pm, const char* name)
{
    int i;
    for (i = 0; i < pm->_PagePoolN; i++)
    {
        if (strcmp(name, pm->_PagePool[i]->_Name) == 0)
        {
            return pm->_PagePool[i];
        }
    }
    return NULL;
}

/**
  * @brief  Search pages in the page stack
  * @param  name: Page name
  * @retval A pointer to the base class of the page, or NULL if not found
  */
PageBase* PageManager_FindPageInStack(PageManager* pm, const char* name)
{
    int i;
    for (i = pm->_PageStackN - 1; i >= 0; i--)
    {
        if (strcmp(name, pm->_PageStack[i]->_Name) == 0)
        {
            return pm->_PageStack[i];
        }
    }
    return NULL;
}

/**
  * @brief  Install the page, and register the page to the page pool
  * @param  className: The class name of the page
  * @param  appName: Page application name, no duplicates allowed
  * @retval Return true if successful
  */
bool PageManager_Install(PageManager* pm, const char* className, const char* appName)
{
    if (pm->_CreatePage == NULL)
    {
        PM_LOG_ERROR("Factory was not registered, can't install page");
        return false;
    }

    if (appName == NULL)
    {
        PM_LOG_WARN("appName has not set");
        appName = className;
    }

    if (PageManager_FindPageInPool(pm, appName) != NULL)
    {
        PM_LOG_ERROR("Page(%s) was registered", appName);
        return false;
    }

    PageBase* base = pm->_CreatePage(className);
    if (base == NULL)
    {
        PM_LOG_ERROR("Factory has not %s", className);
        return false;
    }

    base->_root = NULL;
    base->_ID = 0;
    base->_Manager = NULL;
    base->_UserData = NULL;
    memset(&base->priv, 0, sizeof(base->priv));

    PM_LOG_INFO("Install Page[class = %s, name = %s]", className, appName);
    bool retval = PageManager_Register(pm, base, appName);

    PAGE_CALL(base, on_custom_attr);

    return retval;
}

/**
  * @brief  Uninstall page
  * @param  appName: Page application name, no duplicates allowed
  * @retval Return true if the uninstallation is successful
  */
bool PageManager_Uninstall(PageManager* pm, const char* appName)
{
    PM_LOG_INFO("Page(%s) uninstall...", appName);

    PageBase* base = PageManager_FindPageInPool(pm, appName);
    if (base == NULL)
    {
        PM_LOG_ERROR("Page(%s) was not found", appName);
        return false;
    }

    if (!PageManager_Unregister(pm, appName))
    {
        PM_LOG_ERROR("Page(%s) unregister failed", appName);
        return false;
    }

    if (base->priv.IsCached)
    {
        PM_LOG_WARN("Page(%s) has cached, unloading...", appName);
        base->priv.State = PAGE_STATE_UNLOAD;
        PageManager_StateUpdate(pm, base);
    }
    else
    {
        PM_LOG_INFO("Page(%s) has not cache", appName);
    }

    PAGE_CALL(base, destroy);
    PM_LOG_INFO("Uninstall OK");
    return true;
}

/**
  * @brief  Register the page to the page pool
  * @param  name: Page application name, duplicate registration is not allowed
  * @retval Return true if the registration is successful
  */
bool PageManager_Register(PageManager* pm, PageBase* base, const char* name)
{
    if (PageManager_FindPageInPool(pm, name) != NULL)
    {
        PM_LOG_ERROR("Page(%s) was multi registered", name);
        return false;
    }

    if (pm->_PagePoolN >= PAGE_POOL_MAX)
    {
        PM_LOG_ERROR("Page pool is full");
        return false;
    }

    base->_Manager = pm;
    base->_Name = name;

    pm->_PagePool[pm->_PagePoolN++] = base;

    return true;
}

/**
  * @brief  Log out the page from the page pool
  * @param  name: Page application name
  * @retval Return true if the logout is successful
  */
bool PageManager_Unregister(PageManager* pm, const char* name)
{
    PM_LOG_INFO("Page(%s) unregister...", name);

    PageBase* base = PageManager_FindPageInStack(pm, name);

    if (base != NULL)
    {
        PM_LOG_ERROR("Page(%s) was in stack", name);
        return false;
    }

    base = PageManager_FindPageInPool(pm, name);
    if (base == NULL)
    {
        PM_LOG_ERROR("Page(%s) was not found", name);
        return false;
    }

    int i;
    for (i = 0; i < pm->_PagePoolN; i++)
    {
        if (pm->_PagePool[i] == base)
        {
            int j;
            for (j = i; j < pm->_PagePoolN - 1; j++)
            {
                pm->_PagePool[j] = pm->_PagePool[j + 1];
            }
            pm->_PagePoolN--;
            PM_LOG_INFO("Unregister OK");
            return true;
        }
    }

    PM_LOG_ERROR("Page(%s) was not found in PagePool", name);
    return false;
}

/**
  * @brief  Get the top page of the page stack
  * @param  None
  * @retval A pointer to the base class of the page
  */
PageBase* PageManager_GetStackTop(PageManager* pm)
{
    return (pm->_PageStackN <= 0) ? NULL : pm->_PageStack[pm->_PageStackN - 1];
}

/**
  * @brief  Get the page below the top of the page stack
  * @param  None
  * @retval A pointer to the base class of the page
  */
PageBase* PageManager_GetStackTopAfter(PageManager* pm)
{
    return (pm->_PageStackN < 2) ? NULL : pm->_PageStack[pm->_PageStackN - 2];
}

/**
  * @brief  Clear the page stack and end the life cycle of all pages in the page stack
  * @param  keepBottom: Whether to keep the bottom page of the stack
  * @retval None
  */
void PageManager_SetStackClear(PageManager* pm, bool keepBottom)
{
    while (1)
    {
        PageBase* top = PageManager_GetStackTop(pm);

        if (top == NULL)
        {
            PM_LOG_INFO("Page stack is empty, breaking...");
            break;
        }

        PageBase* topAfter = PageManager_GetStackTopAfter(pm);

        if (topAfter == NULL)
        {
            if (keepBottom)
            {
                pm->_PagePrev = top;
                PM_LOG_INFO("Keep page stack bottom(%s), breaking...", top->_Name);
                break;
            }
            else
            {
                pm->_PagePrev = NULL;
            }
        }

        PageManager_FourceUnload(pm, top);

        pm->_PageStackN--;
    }
    PM_LOG_INFO("Stack clear done");
}

/**
  * @brief  Get the name of the previous page
  * @param  None
  * @retval The name of the previous page, if it does not exist, return PM_EMPTY_PAGE_NAME
  */
const char* PageManager_GetPagePrevName(PageManager* pm)
{
    return pm->_PagePrev ? pm->_PagePrev->_Name : PM_EMPTY_PAGE_NAME;
}

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
PageManager::PageManager(PageCreateFn create)
    : _CreatePage(create)
    , _PagePoolN(0)
    , _PageStackN(0)
    , _PagePrev(nullptr)
    , _PageCurrent(nullptr)
    , _RootDefaultStyle(nullptr)
{
    memset(&_AnimState, 0, sizeof(_AnimState));

    SetGlobalLoadAnimType();
}

/**
  * @brief  Page manager destructor
  * @param  None
  * @retval None
  */
PageManager::~PageManager()
{
    SetStackClear();
}

/**
  * @brief  Search pages in the page pool
  * @param  name: Page name
  * @retval A pointer to the base class of the page, or nullptr if not found
  */
PageBase* PageManager::FindPageInPool(const char* name)
{
    int i;
    for (i = 0; i < _PagePoolN; i++)
    {
        if (strcmp(name, _PagePool[i]->_Name) == 0)
        {
            return _PagePool[i];
        }
    }
    return nullptr;
}

/**
  * @brief  Search pages in the page stack
  * @param  name: Page name
  * @retval A pointer to the base class of the page, or nullptr if not found
  */
PageBase* PageManager::FindPageInStack(const char* name)
{
    int i;
    for (i = _PageStackN - 1; i >= 0; i--)
    {
        if (strcmp(name, _PageStack[i]->_Name) == 0)
        {
            return _PageStack[i];
        }
    }
    return nullptr;
}

/**
  * @brief  Install the page, and register the page to the page pool
  * @param  className: The class name of the page
  * @param  appName: Page application name, no duplicates allowed
  * @retval Return true if successful
  */
bool PageManager::Install(const char* className, const char* appName)
{
    if (_CreatePage == nullptr)
    {
        PM_LOG_ERROR("Factory was not registered, can't install page");
        return false;
    }

    if (appName == nullptr)
    {
        PM_LOG_WARN("appName has not set");
        appName = className;
    }

    if (FindPageInPool(appName) != nullptr)
    {
        PM_LOG_ERROR("Page(%s) was registered", appName);
        return false;
    }

    PageBase* base = _CreatePage(className);
    if (base == nullptr)
    {
        PM_LOG_ERROR("Factory has not %s", className);
        return false;
    }

    base->_root = nullptr;
    base->_ID = 0;
    base->_Manager = nullptr;
    base->_UserData = nullptr;
    memset(&base->priv, 0, sizeof(base->priv));

    PM_LOG_INFO("Install Page[class = %s, name = %s]", className, appName);
    bool retval = Register(base, appName);

    PAGE_CALL(base, on_custom_attr);

    return retval;
}

/**
  * @brief  Uninstall page
  * @param  appName: Page application name, no duplicates allowed
  * @retval Return true if the uninstallation is successful
  */
bool PageManager::Uninstall(const char* appName)
{
    PM_LOG_INFO("Page(%s) uninstall...", appName);

    PageBase* base = FindPageInPool(appName);
    if (base == nullptr)
    {
        PM_LOG_ERROR("Page(%s) was not found", appName);
        return false;
    }

    if (!Unregister(appName))
    {
        PM_LOG_ERROR("Page(%s) unregister failed", appName);
        return false;
    }

    if (base->priv.IsCached)
    {
        PM_LOG_WARN("Page(%s) has cached, unloading...", appName);
        base->priv.State = PAGE_STATE_UNLOAD;
        StateUpdate(base);
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
bool PageManager::Register(PageBase* base, const char* name)
{
    if (FindPageInPool(name) != nullptr)
    {
        PM_LOG_ERROR("Page(%s) was multi registered", name);
        return false;
    }

    if (_PagePoolN >= PAGE_POOL_MAX)
    {
        PM_LOG_ERROR("Page pool is full");
        return false;
    }

    base->_Manager = this;
    base->_Name = name;

    _PagePool[_PagePoolN++] = base;

    return true;
}

/**
  * @brief  Log out the page from the page pool
  * @param  name: Page application name
  * @retval Return true if the logout is successful
  */
bool PageManager::Unregister(const char* name)
{
    PM_LOG_INFO("Page(%s) unregister...", name);

    PageBase* base = FindPageInStack(name);

    if (base != nullptr)
    {
        PM_LOG_ERROR("Page(%s) was in stack", name);
        return false;
    }

    base = FindPageInPool(name);
    if (base == nullptr)
    {
        PM_LOG_ERROR("Page(%s) was not found", name);
        return false;
    }

    int i;
    for (i = 0; i < _PagePoolN; i++)
    {
        if (_PagePool[i] == base)
        {
            int j;
            for (j = i; j < _PagePoolN - 1; j++)
            {
                _PagePool[j] = _PagePool[j + 1];
            }
            _PagePoolN--;
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
PageBase* PageManager::GetStackTop()
{
    return (_PageStackN <= 0) ? nullptr : _PageStack[_PageStackN - 1];
}

/**
  * @brief  Get the page below the top of the page stack
  * @param  None
  * @retval A pointer to the base class of the page
  */
PageBase* PageManager::GetStackTopAfter()
{
    return (_PageStackN < 2) ? nullptr : _PageStack[_PageStackN - 2];
}

/**
  * @brief  Clear the page stack and end the life cycle of all pages in the page stack
  * @param  keepBottom: Whether to keep the bottom page of the stack
  * @retval None
  */
void PageManager::SetStackClear(bool keepBottom)
{
    while (1)
    {
        PageBase* top = GetStackTop();

        if (top == nullptr)
        {
            PM_LOG_INFO("Page stack is empty, breaking...");
            break;
        }

        PageBase* topAfter = GetStackTopAfter();

        if (topAfter == nullptr)
        {
            if (keepBottom)
            {
                _PagePrev = top;
                PM_LOG_INFO("Keep page stack bottom(%s), breaking...", top->_Name);
                break;
            }
            else
            {
                _PagePrev = nullptr;
            }
        }

        FourceUnload(top);

        _PageStackN--;
    }
    PM_LOG_INFO("Stack clear done");
}

/**
  * @brief  Get the name of the previous page
  * @param  None
  * @retval The name of the previous page, if it does not exist, return PM_EMPTY_PAGE_NAME
  */
const char* PageManager::GetPagePrevName()
{
    return _PagePrev ? _PagePrev->_Name : PM_EMPTY_PAGE_NAME;
}

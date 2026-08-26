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
#include "AppFactory.h"
#include "_Template/Template.h"
#include "LiveMap/LiveMap.h"
#include "Dialplate/Dialplate.h"
#include "SystemInfos/SystemInfos.h"
#include "StartUp/StartUp.h"

/**
  * @brief  按类名创建页面对象
  * @param  name  类名字符串，与 Install 的第一个参数相同，如 "Dialplate"
  * @note   五页都是 C 静态单例 Xxx_Create()。不是应用名 "Pages/Dialplate"。
  * @retval 新页面的 PageBase*；未登记则 nullptr
  */
PageBase* AppFactory::CreatePage(const char* name)
{
    if (strcmp(name, "Template") == 0)
    {
        return Template_Create();
    }
    if (strcmp(name, "Startup") == 0)
    {
        return Startup_Create();
    }
    if (strcmp(name, "Dialplate") == 0)
    {
        return Dialplate_Create();
    }
    if (strcmp(name, "SystemInfos") == 0)
    {
        return SystemInfos_Create();
    }
    if (strcmp(name, "LiveMap") == 0)
    {
        return LiveMap_Create();
    }

    return nullptr;
}

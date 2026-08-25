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
  * @brief  按类名匹配并 new 出对应页面
  * @param  className  标识符，如 Dialplate；#className 变成字符串 "Dialplate"
  * @note   必须写成宏：new 的类型名在编译期拼出来，C 里就是一张函数表。
  *         do { ... } while(0) 让宏当一条语句用，避免 if 里少写花括号出错。
  */
#define APP_CLASS_MATCH(className)\
do{\
    if (strcmp(name, #className) == 0)\
    {\
        return new Page::className;\
    }\
}while(0)

/**
  * @brief  按类名创建页面对象
  * @param  name  类名字符串，与 Install 的第一个参数相同，如 "Dialplate"
  * @note   不是应用名 "Pages/Dialplate"。新加一页：include 头文件 + 下面加一行 APP_CLASS_MATCH。
  * @retval 新页面的 PageBase*；未登记则 nullptr
  */
PageBase* AppFactory::CreatePage(const char* name)
{
    APP_CLASS_MATCH(Template);
    APP_CLASS_MATCH(LiveMap);
    APP_CLASS_MATCH(Dialplate);
    APP_CLASS_MATCH(SystemInfos);
    APP_CLASS_MATCH(Startup);

    return nullptr;
}

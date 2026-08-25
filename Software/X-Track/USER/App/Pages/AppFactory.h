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
#include "Utils/PageManager/PageManager.h"

/**
  * @brief  本工程的页面工厂：按类名 new 出具体页面
  * @note   继承 PageFactory，交给 PageManager 构造时传入。
  *         C 对应一张 { "Dialplate", Dialplate_Create } 函数表。
  *         Install("Dialplate", "Pages/Dialplate") 时会调 CreatePage("Dialplate")。
  */
class AppFactory : public PageFactory
{
public:
    /**
      * @brief  按类名创建页面对象
      * @param  name  类名字符串，如 "Dialplate" / "Startup"（不是 "Pages/Dialplate"）
      * @note   实现见 AppFactory.cpp 的 APP_CLASS_MATCH：strcmp 命中则 new Page::Xxx。
      *         未登记的名字返回 nullptr，Install 会失败。
      * @retval 新页面的 PageBase*；找不到则 nullptr
      */
    virtual PageBase* CreatePage(const char* name);
private:

};


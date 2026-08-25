#ifndef __TEMPLATE_MODEL_H
#define __TEMPLATE_MODEL_H

#include "lvgl/lvgl.h"

namespace Page
{

/**
  * @brief  模板页 Model：示范「可以极瘦」
  * @note   没有 DataCenter 账号，不 Subscribe。正式页面应在这里 Pull/Notify。
  *         C 对应 struct { uint32_t tick_save; } + GetData()。
  */
class TemplateModel
{
public:
    uint32_t TickSave;   /**< 进入页面时记下的 tick，onViewLoad 里赋值 */

    /**
      * @brief  取当前 LVGL 毫秒 tick
      * @note   实现就是 return lv_tick_get()。正式页应改成问总线要数据。
      * @retval 开机以来的毫秒数
      */
    uint32_t GetData();
private:

};

}

#endif

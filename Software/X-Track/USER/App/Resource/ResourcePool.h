#ifndef __RESOURCE_POOL
#define __RESOURCE_POOL

#include "lvgl/lvgl.h"

#ifdef __cplusplus
namespace ResourcePool
{

void Init();
lv_font_t* GetFont(const char* name);
const void* GetImage(const char* name);

}

extern "C" {
#endif

/** C 页面用：转调 ResourcePool::GetFont */
lv_font_t* ResourcePool_GetFont(const char* name);
/** C 页面用：转调 ResourcePool::GetImage */
const void* ResourcePool_GetImage(const char* name);

#ifdef __cplusplus
}
#endif

#endif

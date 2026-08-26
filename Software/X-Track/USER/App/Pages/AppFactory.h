#ifndef __APP_FACTORY_H
#define __APP_FACTORY_H

#include "Utils/PageManager/PageBase.h"

#ifdef __cplusplus
extern "C" {
#endif

PageBase* AppFactory_CreatePage(const char* name);

#ifdef __cplusplus
}
#endif

#endif

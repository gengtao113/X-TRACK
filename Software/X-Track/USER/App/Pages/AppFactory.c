#include "AppFactory.h"
#include "_Template/Template.h"
#include "LiveMap/LiveMap.h"
#include "Dialplate/Dialplate.h"
#include "SystemInfos/SystemInfos.h"
#include "StartUp/StartUp.h"
#include <stddef.h>
#include <string.h>

typedef PageBase* (*PageCreateFn)(void);

static const struct
{
    const char* class_name;
    PageCreateFn create;
} k_page_table[] = {
    { "Template",    Template_Create },
    { "Startup",     Startup_Create },
    { "Dialplate",   Dialplate_Create },
    { "SystemInfos", SystemInfos_Create },
    { "LiveMap",     LiveMap_Create },
};

PageBase* AppFactory_CreatePage(const char* name)
{
    size_t i;

    if (name == NULL)
    {
        return NULL;
    }

    for (i = 0; i < sizeof(k_page_table) / sizeof(k_page_table[0]); i++)
    {
        if (strcmp(name, k_page_table[i].class_name) == 0)
        {
            return k_page_table[i].create();
        }
    }

    return NULL;
}

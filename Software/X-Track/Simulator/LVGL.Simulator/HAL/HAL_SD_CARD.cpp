#include "HAL.h"

bool SD_Init()
{
    return true;
}

bool SD_GetReady()
{
    return true;
}

float SD_GetCardSizeMB()
{
    return 32 * 1024;
}

const char* SD_GetTypeName()
{
    return "SDHC";
}

static void SD_Check(bool isInsert)
{
   
}

void SD_SetEventCallback(SD_CallbackFunction_t callback)
{
    
}

void SD_Update()
{
    
}

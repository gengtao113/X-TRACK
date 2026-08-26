#include "HAL.h"
#include "hal_dev.h"

static CommitFunc_t CommitFunc = NULL;
static void* UserData = NULL;

bool MAG_Init(void)
{
    Serial_Print("MAG: init...");

    bool success = DevMAG_Init();

    Serial_Println(success ? "success" : "failed");

    return success;
}

void MAG_SetCommitCallback(CommitFunc_t func, void* userData)
{
    CommitFunc = func;
    UserData = userData;
}

void MAG_Update(void)
{
    MAG_Info_t magInfo;
    DevMAG_Read(&magInfo);

    if(CommitFunc)
    {
        CommitFunc(&magInfo, UserData);
    }
}

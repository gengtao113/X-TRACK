#include "HAL.h"
#include "hal_dev.h"

static CommitFunc_t CommitFunc = NULL;
static void* UserData = NULL;

bool IMU_Init(void)
{
    Serial_Print("IMU: init...");

    bool success = DevIMU_Init();

    Serial_Println(success ? "success" : "failed");

    return success;
}

void IMU_SetCommitCallback(CommitFunc_t func, void* userData)
{
    CommitFunc = func;
    UserData = userData;
}

void IMU_Update(void)
{
    IMU_Info_t imuInfo;
    DevIMU_GetMotion(&imuInfo);

    if(CommitFunc)
    {
        CommitFunc(&imuInfo, UserData);
    }
}

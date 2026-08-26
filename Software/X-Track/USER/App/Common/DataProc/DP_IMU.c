#include "DataProc.h"
#include "../HAL/hal_c.h"

static bool IMU_OnCommit(void* info, void* userData)
{
    return Account_Commit((Account*)userData, info, sizeof(IMU_Info_t));
}

DATA_PROC_INIT_DEF(IMU)
{
    HAL_IMU_SetCommitCallback(IMU_OnCommit, account);
}

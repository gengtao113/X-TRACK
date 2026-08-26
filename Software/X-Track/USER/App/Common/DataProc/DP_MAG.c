#include "DataProc.h"
#include "../HAL/hal_c.h"

static bool MAG_OnCommit(void* info, void* userData)
{
    return Account_Commit((Account*)userData, info, sizeof(MAG_Info_t));
}

DATA_PROC_INIT_DEF(MAG)
{
    HAL_MAG_SetCommitCallback(MAG_OnCommit, account);
}

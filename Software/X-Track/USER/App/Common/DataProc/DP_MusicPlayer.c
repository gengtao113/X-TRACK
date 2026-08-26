#include "DataProc.h"
#include "../HAL/hal_c.h"

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    MusicPlayer_Info_t* info;

    (void)account;
    (void)from;

    if (event != ACCOUNT_EVENT_NOTIFY)
    {
        return ACCOUNT_RES_UNSUPPORTED_REQUEST;
    }

    if (size != sizeof(MusicPlayer_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    info = (MusicPlayer_Info_t*)data;
    HAL_Audio_PlayMusic(info->music);

    return ACCOUNT_RES_OK;
}

DATA_PROC_INIT_DEF(MusicPlayer)
{
    Account_SetCallback(account, onEvent);
}

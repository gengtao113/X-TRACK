#include "DataProc.h"
#include "../HAL/hal_c.h"
#include "Utils/StorageService/storage_service_c.h"
#include "Utils/MapConv/mapconv_c.h"
#include "Config/Config.h"
#include "dataproc_c.h"
#include <stdlib.h>

#define MAP_LEVEL_MIN    0
#define MAP_LEVEL_MAX    19

static StorageService* storageService;

static bool MapConvGetRange(const char* dirName, int16_t* min, int16_t* max)
{
    bool retval = false;
    lv_fs_dir_t dir;

    if (lv_fs_dir_open(&dir, dirName) == LV_FS_RES_OK)
    {
        LV_LOG_USER("%s open success", dirName);

        int16_t levelMin = MAP_LEVEL_MAX;
        int16_t levelMax = MAP_LEVEL_MIN;

        char name[128];
        while (1)
        {
            lv_fs_res_t res = lv_fs_dir_read(&dir, name);

            if (name[0] == '\0' || res != LV_FS_RES_OK)
            {
                break;
            }

            if (name[0] == '/')
            {
                retval = true;
                int level = atoi(name + 1);

                if (level < MAP_LEVEL_MIN || level > MAP_LEVEL_MAX)
                {
                    LV_LOG_ERROR("Error level = %d", level);
                    retval = false;
                    break;
                }

                if (level < levelMin)
                {
                    levelMin = level;
                }

                if (level > levelMax)
                {
                    levelMax = level;
                }
            }
        }

        if (retval)
        {
            *min = levelMin;
            *max = levelMax;
        }

        lv_fs_dir_close(&dir);
    }
    else
    {
        LV_LOG_ERROR("%s open faild", dirName);
    }
    return retval;
}

static bool onLoad(Account* account)
{
    bool success = StorageService_LoadFile(storageService);
    SysConfig_Info_t sysConfig;
    int16_t levelMin;
    int16_t levelMax;

    if (!success)
    {
        LV_LOG_WARN("Load " CONFIG_SYSTEM_SAVE_FILE_PATH " error");
    }

    if (Account_Pull(account, "SysConfig", &sysConfig, sizeof(sysConfig)) != ACCOUNT_RES_OK)
    {
        LV_LOG_ERROR("Pull SysConfig failed!");
        return false;
    }

    MapConv_SetDirPath(sysConfig.mapDirPath);
    MapConv_SetExtName(sysConfig.mapExtName);
    MapConv_SetCoordTransformEnable(!sysConfig.mapWGS84);

    if (MapConvGetRange(sysConfig.mapDirPath, &levelMin, &levelMax))
    {
        MapConv_SetLevelRange(levelMin, levelMax);
    }
    else
    {
        LV_LOG_ERROR("Get map level range failed!");
    }

    LV_LOG_USER(
        "Map path: %s, WGS84: %d, level min = %d, max = %d",
        sysConfig.mapDirPath,
        sysConfig.mapWGS84,
        MapConv_GetLevelMin(),
        MapConv_GetLevelMax()
    );
    LV_LOG_USER("Map ext name: *.%s", sysConfig.mapExtName);
#if CONFIG_MAP_PNG_DECODE_ENABLE
    LV_LOG_USER("Map PNG decoder enable");
#endif

    return success;
}

static void onNotify(Account* account, Storage_Info_t* info)
{
    static bool isLoadSuccess = false;

    switch (info->cmd)
    {
    case STORAGE_CMD_LOAD:
        isLoadSuccess = onLoad(account);
        break;
    case STORAGE_CMD_SAVE:
        StorageService_SaveFile(storageService, NULL);
        if (isLoadSuccess)
        {
            LV_LOG_USER("Saving backup file: " CONFIG_SYSTEM_SAVE_FILE_BACKUP_PATH);
            StorageService_SaveFile(storageService, CONFIG_SYSTEM_SAVE_FILE_BACKUP_PATH);
        }
        break;
    case STORAGE_CMD_ADD:
        StorageService_Add(
            storageService,
            info->key,
            info->value,
            info->size,
            (int)info->type
        );
        break;
    case STORAGE_CMD_REMOVE:
        StorageService_Remove(storageService, info->key);
        break;
    default:
        break;
    }
}

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    (void)from;

    if (event == ACCOUNT_EVENT_SUB_PULL)
    {
        Storage_Basic_Info_t* info;

        if (size != sizeof(Storage_Basic_Info_t))
        {
            return ACCOUNT_RES_SIZE_MISMATCH;
        }

        info = (Storage_Basic_Info_t*)data;
        info->isDetect = HAL_SD_GetReady();
        info->totalSizeMB = HAL_SD_GetCardSizeMB();
        info->freeSizeMB = 0.0f;
        info->type = HAL_SD_GetTypeName();
        return ACCOUNT_RES_OK;
    }

    if (event != ACCOUNT_EVENT_NOTIFY)
    {
        return ACCOUNT_RES_UNSUPPORTED_REQUEST;
    }

    if (size != sizeof(Storage_Info_t))
    {
        return ACCOUNT_RES_SIZE_MISMATCH;
    }

    onNotify(account, (Storage_Info_t*)data);

    return ACCOUNT_RES_OK;
}

static void onSDEvent(bool insert)
{
    if (insert)
    {
        Storage_Info_t info;
        DATA_PROC_INIT_STRUCT(info);
        info.cmd = STORAGE_CMD_LOAD;
        Account_Notify((Account*)DataProc_MainAccount(), "Storage", &info, sizeof(info));
    }
}

DATA_PROC_INIT_DEF(Storage)
{
    storageService = StorageService_Create(CONFIG_SYSTEM_SAVE_FILE_PATH, 4096);
    Account_SetCallback(account, onEvent);
    Account_Subscribe(account, "SysConfig");
    HAL_SD_SetEventCallback(onSDEvent);
}

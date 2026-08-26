#include <stdio.h>
#include "DataProc.h"
#include "Utils/GPX/gpx_c.h"
#include "Config/Config.h"
#include "Version.h"

#define RECORDER_GPX_TIME_FMT    "%d-%02d-%02dT%02d:%02d:%02dZ"
#define RECORDER_GPX_FILE_NAME   "/" CONFIG_TRACK_RECORD_FILE_DIR_NAME "/TRK_%d%02d%02d_%02d%02d%02d.gpx"
#define RECORDER_GPX_META_NAME   VERSION_FIRMWARE_NAME " " VERSION_SOFTWARE
#define RECORDER_GPX_META_DESC   VERSION_PROJECT_LINK
#define RECORDER_GPX_STR_MAX     2048

typedef struct
{
    GPX* gpx;
    Recorder_Info_t recInfo;
    lv_fs_file_t file;
    bool active;
    Account* account;
} Recorder_t;

static lv_fs_res_t Recorder_FileWriteString(lv_fs_file_t* file_p, const char* str)
{
    lv_fs_res_t res = lv_fs_write(
                          file_p,
                          str,
                          (uint32_t)strlen(str),
                          NULL
                      );

    return res;
}

static void Recorder_WriteGpx(lv_fs_file_t* file_p, void (*get)(GPX*, char*, uint32_t), GPX* gpx)
{
    char buf[RECORDER_GPX_STR_MAX];
    get(gpx, buf, sizeof(buf));
    Recorder_FileWriteString(file_p, buf);
}

static int Recorder_GetTimeConv(
    Recorder_t* recorder,
    const char* format,
    char* buf,
    uint32_t size)
{
    Clock_Info_t clock;
    int retval = -1;
    if (Account_Pull(recorder->account, "Clock", &clock, sizeof(clock)) == ACCOUNT_RES_OK)
    {
        retval = snprintf(
            buf,
            size,
            format,
            clock.year,
            clock.month,
            clock.day,
            clock.hour,
            clock.minute,
            clock.second
        );
    }

    return retval;
}

static void Recorder_RecPoint(Recorder_t* recorder, GPS_Info_t* gpsInfo)
{
    char timeBuf[64];
    char eleBuf[32];
    char lonBuf[32];
    char latBuf[32];
    char gpxBuf[RECORDER_GPX_STR_MAX];
    int ret;

    ret = Recorder_GetTimeConv(
        recorder,
        RECORDER_GPX_TIME_FMT,
        timeBuf,
        sizeof(timeBuf)
    );

    if (ret < 0)
    {
        LV_LOG_WARN("cant't get time");
        return;
    }

    snprintf(eleBuf, sizeof(eleBuf), "%.2f", gpsInfo->altitude);
    snprintf(lonBuf, sizeof(lonBuf), "%.6f", gpsInfo->longitude);
    snprintf(latBuf, sizeof(latBuf), "%.6f", gpsInfo->latitude);

    GPX_SetEle(recorder->gpx, eleBuf);
    GPX_SetTime(recorder->gpx, timeBuf);
    GPX_GetPt(recorder->gpx, GPX_TRKPT, lonBuf, latBuf, gpxBuf, sizeof(gpxBuf));
    Recorder_FileWriteString(&(recorder->file), gpxBuf);
}

static void Recorder_RecStart(Recorder_t* recorder, uint16_t time)
{
    char filepath[128];
    int ret;
    lv_fs_res_t res;

    (void)time;

    LV_LOG_USER("Track record start");

    ret = Recorder_GetTimeConv(
        recorder,
        RECORDER_GPX_FILE_NAME,
        filepath, sizeof(filepath)
    );

    if (ret < 0)
    {
        LV_LOG_WARN("cant't get time");
        return;
    }

    res = lv_fs_open(&(recorder->file), filepath, LV_FS_MODE_WR | LV_FS_MODE_RD);

    if (res == LV_FS_RES_OK)
    {
        GPX* gpx = recorder->gpx;
        lv_fs_file_t* file_p = &(recorder->file);

        LV_LOG_USER("Track file %s open success", filepath);

        GPX_SetMetaName(gpx, RECORDER_GPX_META_NAME);
        GPX_SetMetaDesc(gpx, RECORDER_GPX_META_DESC);
        GPX_SetName(gpx, filepath);
        GPX_SetDesc(gpx, "");

        Recorder_WriteGpx(file_p, GPX_GetOpen, gpx);
        Recorder_WriteGpx(file_p, GPX_GetMetaData, gpx);
        Recorder_WriteGpx(file_p, GPX_GetTrakOpen, gpx);
        Recorder_WriteGpx(file_p, GPX_GetInfo, gpx);
        Recorder_WriteGpx(file_p, GPX_GetTrakSegOpen, gpx);

        recorder->active = true;
    }
    else
    {
        LV_LOG_ERROR("Track file open error!");
    }
}

static void Recorder_RecStop(Recorder_t* recorder)
{
    GPX* gpx = recorder->gpx;
    lv_fs_file_t* file_p = &(recorder->file);

    recorder->active = false;
    Recorder_WriteGpx(file_p, GPX_GetTrakSegClose, gpx);
    Recorder_WriteGpx(file_p, GPX_GetTrakClose, gpx);
    Recorder_WriteGpx(file_p, GPX_GetClose, gpx);
    lv_fs_close(file_p);

    LV_LOG_USER("Track record end");
}

static int onNotify(Recorder_t* recorder, Recorder_Info_t* info)
{
    switch (info->cmd)
    {
    case RECORDER_CMD_START:
        Recorder_RecStart(recorder, info->time);
        break;
    case RECORDER_CMD_PAUSE:
        recorder->active = false;
        LV_LOG_USER("Track record pause");
        break;
    case RECORDER_CMD_CONTINUE:
        LV_LOG_USER("Track record continue");
        recorder->active = true;
        break;
    case RECORDER_CMD_STOP:
        Recorder_RecStop(recorder);
        break;
    }

    {
        TrackFilter_Info_t tfInfo;
        DATA_PROC_INIT_STRUCT(tfInfo);
        tfInfo.cmd = (TrackFilter_Cmd_t)info->cmd;
        return Account_Notify(recorder->account, "TrackFilter", &tfInfo, sizeof(tfInfo));
    }
}

static int onEvent(Account* account, int event, void* from, void* data, uint32_t size)
{
    int res = ACCOUNT_RES_UNKNOW;
    Recorder_t* recorder = (Recorder_t*)account->UserData;

    (void)from;

    switch (event)
    {
    case ACCOUNT_EVENT_PUB_PUBLISH:
        if (size == sizeof(GPS_Info_t))
        {
            if (recorder->active)
            {
                Recorder_RecPoint(recorder, (GPS_Info_t*)data);
            }
            res = ACCOUNT_RES_OK;
        }
        else
        {
            res = ACCOUNT_RES_SIZE_MISMATCH;
        }
        break;

    case ACCOUNT_EVENT_SUB_PULL:
        if (size == sizeof(Recorder_Info_t))
        {
            memcpy(data, &(recorder->recInfo), size);
        }
        else
        {
            res = ACCOUNT_RES_SIZE_MISMATCH;
        }
        break;

    case ACCOUNT_EVENT_NOTIFY:
        if (size == sizeof(Recorder_Info_t))
        {
            onNotify(recorder, (Recorder_Info_t*)data);
            res = ACCOUNT_RES_OK;
        }
        else
        {
            res = ACCOUNT_RES_SIZE_MISMATCH;
        }
        break;

    default:
        break;
    }

    return res;
}

DATA_PROC_INIT_DEF(Recorder)
{
    static Recorder_t recorder;
    memset(&recorder.recInfo, 0, sizeof(recorder.recInfo));
    memset(&recorder.file, 0, sizeof(recorder.file));
    recorder.active = false;
    recorder.account = account;
    recorder.gpx = GPX_Create();
    account->UserData = &recorder;

    Account_Subscribe(account, "GPS");
    Account_Subscribe(account, "Clock");
    Account_Subscribe(account, "TrackFilter");
    Account_SetCallback(account, onEvent);
}

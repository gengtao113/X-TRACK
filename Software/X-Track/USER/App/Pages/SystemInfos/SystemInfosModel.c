#include "SystemInfosModel.h"
#include "Common/DataProc/dataproc_c.h"
#include "Common/HAL/HAL_Def.h"
#include <stdio.h>
#include <string.h>

void SystemInfosModel_Init(SystemInfosModel* m)
{
    m->account = Account_Create("SystemInfosModel", DataProc_Center(), 0, m);

    Account_Subscribe(m->account, "SportStatus");
    Account_Subscribe(m->account, "GPS");
    Account_Subscribe(m->account, "MAG");
    Account_Subscribe(m->account, "IMU");
    Account_Subscribe(m->account, "Clock");
    Account_Subscribe(m->account, "Power");
    Account_Subscribe(m->account, "Storage");
    Account_Subscribe(m->account, "StatusBar");
}

void SystemInfosModel_Deinit(SystemInfosModel* m)
{
    if (m->account)
    {
        Account_Destroy(m->account);
        m->account = NULL;
    }
}

void SystemInfosModel_GetSportInfo(SystemInfosModel* m, float* trip, char* time, uint32_t len, float* maxSpd)
{
    SportStatus_Info_t sport;
    memset(&sport, 0, sizeof(sport));
    Account_Pull(m->account, "SportStatus", &sport, sizeof(sport));
    *trip = sport.totalDistance / 1000;
    DataProc_MakeTimeString(sport.totalTime, time, (uint16_t)len);
    *maxSpd = sport.speedMaxKph;
}

void SystemInfosModel_GetGPSInfo(SystemInfosModel* m, float* lat, float* lng, float* alt, char* utc, uint32_t len, float* course, float* speed)
{
    GPS_Info_t gps;
    memset(&gps, 0, sizeof(gps));
    Account_Pull(m->account, "GPS", &gps, sizeof(gps));
    *lat = (float)gps.latitude;
    *lng = (float)gps.longitude;
    *alt = gps.altitude;
    snprintf(
        utc, len,
        "%d-%d-%d\n%02d:%02d:%02d",
        gps.clock.year,
        gps.clock.month,
        gps.clock.day,
        gps.clock.hour,
        gps.clock.minute,
        gps.clock.second
    );
    *course = gps.course;
    *speed = gps.speed;
}

void SystemInfosModel_GetMAGInfo(SystemInfosModel* m, float* dir, int* x, int* y, int* z)
{
    MAG_Info_t mag;
    memset(&mag, 0, sizeof(mag));
    Account_Pull(m->account, "MAG", &mag, sizeof(mag));
    *dir = 0;
    *x = mag.x;
    *y = mag.y;
    *z = mag.z;
}

void SystemInfosModel_GetIMUInfo(SystemInfosModel* m, int* step, char* info, uint32_t len)
{
    IMU_Info_t imu;
    memset(&imu, 0, sizeof(imu));
    Account_Pull(m->account, "IMU", &imu, sizeof(imu));
    *step = imu.steps;
    snprintf(
        info,
        len,
        "%d\n%d\n%d\n%d\n%d\n%d",
        imu.ax,
        imu.ay,
        imu.az,
        imu.gx,
        imu.gy,
        imu.gz
    );
}

void SystemInfosModel_GetRTCInfo(SystemInfosModel* m, char* dateTime, uint32_t len)
{
    Clock_Info_t clock;
    memset(&clock, 0, sizeof(clock));
    Account_Pull(m->account, "Clock", &clock, sizeof(clock));
    snprintf(
        dateTime,
        len,
        "%d-%d-%d\n%02d:%02d:%02d",
        clock.year,
        clock.month,
        clock.day,
        clock.hour,
        clock.minute,
        clock.second
    );
}

void SystemInfosModel_GetBatteryInfo(SystemInfosModel* m, int* usage, float* voltage, char* state, uint32_t len)
{
    Power_Info_t power;
    memset(&power, 0, sizeof(power));
    Account_Pull(m->account, "Power", &power, sizeof(power));
    *usage = power.usage;
    *voltage = power.voltage / 1000.0f;
    strncpy(state, power.isCharging ? "CHARGE" : "DISCHARGE", len);
    state[len - 1] = '\0';
}

void SystemInfosModel_GetStorageInfo(SystemInfosModel* m, bool* detect, const char** type, char* size, uint32_t len)
{
    Storage_Basic_Info_t info;
    memset(&info, 0, sizeof(info));
    Account_Pull(m->account, "Storage", &info, sizeof(info));
    *detect = info.isDetect;
    *type = info.type;
    snprintf(
        size, len,
        "%0.1f GB",
        info.totalSizeMB / 1024.0f
    );
}

void SystemInfosModel_SetStatusBarStyle(SystemInfosModel* m, StatusBar_Style_t style)
{
    StatusBar_Info_t info;
    memset(&info, 0, sizeof(info));
    info.cmd = STATUS_BAR_CMD_SET_STYLE;
    info.param.style = style;
    Account_Notify(m->account, "StatusBar", &info, sizeof(info));
}

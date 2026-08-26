#include "HAL.h"
#include "App/Version.h"
#include "MillisTaskManager/MillisTaskManager.h"

static MillisTaskManager taskManager;

#if CONFIG_SENSOR_ENABLE

static void HAL_Sensor_Init(void)
{
    if(I2C_Scan() <= 0)
    {
        Serial_Println("I2C: disable sensors");
        return;
    }

#if CONFIG_SENSOR_IMU_ENABLE
    if(IMU_Init())
    {
        MTM_Register(&taskManager, IMU_Update, 1000, true);
    }
#endif

#if CONFIG_SENSOR_MAG_ENABLE
    if(MAG_Init())
    {
        MTM_Register(&taskManager, MAG_Update, 1000, true);
    }
#endif
}

#endif

static void HAL_TimerInterrputUpdate(void)
{
    Power_Update();
    Encoder_Update();
    Audio_Update();
}

void HAL_Init(void)
{
    MTM_Init(&taskManager, false);

    Serial_Begin(115200);
    Serial_Println(VERSION_FIRMWARE_NAME);
    Serial_Println("Version: " VERSION_SOFTWARE);
    Serial_Println("Author: "  VERSION_AUTHOR_NAME);
    Serial_Println("Project: " VERSION_PROJECT_LINK);

    FaultHandle_Init();

    Memory_DumpInfo();

    Power_Init();
    Backlight_Init();
    Encoder_Init();
    Clock_Init();
    Buzz_init();
    GPS_Init();
#if CONFIG_SENSOR_ENABLE
    HAL_Sensor_Init();
#endif
    Audio_Init();
    SD_Init();

    Display_Init();

#if CONFIG_WATCH_DOG_ENABLE
    {
        uint32_t timeout = WDG_Init(CONFIG_WATCH_DOG_TIMEOUT);
        MTM_Register(&taskManager, WDG_ReloadCounter, CONFIG_WATCH_DOG_TIMEOUT / 10, true);
        Serial_Printf("WatchDog: Timeout = %dms\r\n", timeout);
    }
#endif

    MTM_Register(&taskManager, Power_EventMonitor, 100, true);
    MTM_Register(&taskManager, GPS_Update, 200, true);
    MTM_Register(&taskManager, SD_Update, 500, true);
    MTM_Register(&taskManager, Memory_DumpInfo, 1000, true);

    Timer_SetInterrupt(CONFIG_HAL_UPDATE_TIM, 10 * 1000, HAL_TimerInterrputUpdate);
    Timer_SetEnable(CONFIG_HAL_UPDATE_TIM, true);
}

void HAL_Update(void)
{
    MTM_Running(&taskManager, millis());
}

#include "HAL.h"
#include "App/Version.h"
#include "cm_backtrace/cm_backtrace.h"
#include <stdio.h>
#include <stdarg.h>

static void Delay(uint32_t ms)
{
    volatile uint32_t i = F_CPU / 1000 * ms / 5;
    while(i--) {}
}

static void Reboot(void)
{
    while(digitalRead(CONFIG_ENCODER_PUSH_PIN) == HIGH)
    {
        Delay(1000);
    }
    NVIC_SystemReset();
}

void FaultHandle_Init(void)
{
    cm_backtrace_init(
        VERSION_FIRMWARE_NAME,
        VERSION_HARDWARE,
        VERSION_SOFTWARE " " __DATE__
    );
}

void cmb_printf(const char *__restrict __format, ...)
{
    char printf_buff[256];

    va_list args;
    va_start(args, __format);
    vsnprintf(printf_buff, sizeof(printf_buff), __format, args);
    va_end(args);

    Serial_Print(printf_buff);
}

void vApplicationHardFaultHook(void)
{
    Display_DumpCrashInfo("FXXK HardFault!");
    Reboot();
}

#ifdef __CC_ARM
__asm void HardFault_Handler(void)
{
    extern vApplicationHardFaultHook
    extern cm_backtrace_fault

    mov r0, lr
    mov r1, sp
    bl cm_backtrace_fault
    bl vApplicationHardFaultHook
fault_loop
    b fault_loop
}
#endif

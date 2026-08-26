#include "HAL.h"

static bool IsEnable = true;

void Buzz_init(void)
{
    pinMode(CONFIG_BUZZ_PIN, OUTPUT);
}

void Buzz_SetEnable(bool en)
{
    if(!en)
    {
        Buzz_Tone(0, -1);
    }

    IsEnable = en;
}

void Buzz_Tone(uint32_t freq, int32_t duration)
{
    if(!IsEnable)
    {
        return;
    }

    Arduino_Tone(CONFIG_BUZZ_PIN, freq, duration);
}

#include "HAL.h"
#include "hal_dev.h"
#include <string.h>
#include "App/Common/Music/MusicCode.h"

static void Tone_Callback(uint32_t freq, uint16_t volume)
{
    (void)volume;
    Buzz_Tone(freq, -1);
}

void Audio_Init(void)
{
    DevTone_SetCallback(Tone_Callback);
}

void Audio_Update(void)
{
    DevTone_Update(millis());
}

bool Audio_PlayMusic(const char* name)
{
    bool retval = false;
    size_t i;
    for (i = 0; i < sizeof(MusicList) / sizeof(MusicList[0]); i++)
    {
        if (strcmp(name, MusicList[i].name) == 0)
        {
            DevTone_Play(MusicList[i].mc, MusicList[i].length);
            retval = true;
            break;
        }
    }
    return retval;
}

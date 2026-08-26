#ifndef __TONE_PLAYER_TYPES_H
#define __TONE_PLAYER_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t Freq;
    uint16_t Time;
    uint16_t Volume;
} TonePlayer_MusicNode_t;

#ifdef __cplusplus
}
#endif

#endif

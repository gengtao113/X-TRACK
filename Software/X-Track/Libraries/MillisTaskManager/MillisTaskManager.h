#ifndef __MILLISTASKMANAGER_H
#define __MILLISTASKMANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*MTM_TaskFunction_t)(void);

typedef struct MTM_Task
{
    bool State;
    MTM_TaskFunction_t Function;
    uint32_t Time;
    uint32_t TimePrev;
    uint32_t TimeCost;
    uint32_t TimeError;
    struct MTM_Task* Next;
} MTM_Task_t;

typedef struct
{
    MTM_Task_t* Head;
    MTM_Task_t* Tail;
    bool PriorityEnable;
} MillisTaskManager;

void MTM_Init(MillisTaskManager* m, bool priorityEnable);
void MTM_Deinit(MillisTaskManager* m);
MTM_Task_t* MTM_Register(MillisTaskManager* m, MTM_TaskFunction_t func, uint32_t timeMs, bool state);
void MTM_Running(MillisTaskManager* m, uint32_t tick);

#ifdef __cplusplus
}
#endif

#endif

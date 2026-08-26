#include "MillisTaskManager.h"
#include <string.h>

#define TASK_NEW(task) do{ task = (MTM_Task_t*)malloc(sizeof(MTM_Task_t)); }while(0)
#define TASK_DEL(task) do{ free(task); }while(0)

static uint32_t MTM_GetTickElaps(uint32_t nowTick, uint32_t prevTick)
{
    if (nowTick >= prevTick)
    {
        return nowTick - prevTick;
    }
    return (0xFFFFFFFFu - prevTick) + 1u + nowTick;
}

static MTM_Task_t* MTM_Find(MillisTaskManager* m, MTM_TaskFunction_t func)
{
    MTM_Task_t* now = m->Head;
    while (now != NULL)
    {
        if (now->Function == func)
        {
            return now;
        }
        now = now->Next;
    }
    return NULL;
}

void MTM_Init(MillisTaskManager* m, bool priorityEnable)
{
    memset(m, 0, sizeof(*m));
    m->PriorityEnable = priorityEnable;
}

void MTM_Deinit(MillisTaskManager* m)
{
    MTM_Task_t* now = m->Head;
    while (now != NULL)
    {
        MTM_Task_t* now_del = now;
        now = now->Next;
        TASK_DEL(now_del);
    }
    m->Head = NULL;
    m->Tail = NULL;
}

MTM_Task_t* MTM_Register(MillisTaskManager* m, MTM_TaskFunction_t func, uint32_t timeMs, bool state)
{
    MTM_Task_t* task = MTM_Find(m, func);
    if (task != NULL)
    {
        task->Time = timeMs;
        task->State = state;
        return task;
    }

    TASK_NEW(task);
    if (task == NULL)
    {
        return NULL;
    }

    task->Function = func;
    task->Time = timeMs;
    task->State = state;
    task->TimePrev = 0;
    task->TimeCost = 0;
    task->TimeError = 0;
    task->Next = NULL;

    if (m->Head == NULL)
    {
        m->Head = task;
    }
    else
    {
        m->Tail->Next = task;
    }

    m->Tail = task;
    return task;
}

void MTM_Running(MillisTaskManager* m, uint32_t tick)
{
    MTM_Task_t* now = m->Head;
    while (now != NULL)
    {
        if (now->Function != NULL && now->State)
        {
            uint32_t elapsTime = MTM_GetTickElaps(tick, now->TimePrev);
            if (elapsTime >= now->Time)
            {
                now->TimeError = elapsTime - now->Time;
                now->TimePrev = tick;
                now->Function();
                if (m->PriorityEnable)
                {
                    break;
                }
            }
        }
        now = now->Next;
    }
}

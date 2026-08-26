#ifndef __DIALPLATE_PRESENTER_H
#define __DIALPLATE_PRESENTER_H

#include "DialplateView.h"
#include "DialplateModel.h"
#include "Utils/PageManager/PageBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DIALPLATE_RECORD_STATE_READY,
    DIALPLATE_RECORD_STATE_RUN,
    DIALPLATE_RECORD_STATE_PAUSE,
    DIALPLATE_RECORD_STATE_STOP
} DialplateRecordState_t;

typedef struct
{
    PageBase base;
    DialplateView view;
    DialplateModel model;
    lv_timer_t* timer;
    DialplateRecordState_t rec_state;
    lv_obj_t* last_focus;
} DialplatePage;

PageBase* Dialplate_Create(void);

#ifdef __cplusplus
}
#endif

#endif

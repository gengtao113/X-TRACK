#ifndef __ACCOUNT_H
#define __ACCOUNT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "PingPongBuffer/PingPongBuffer.h"
#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ACCOUNT_LIST_MAX
#define ACCOUNT_LIST_MAX 32
#endif

typedef struct Account Account;
typedef struct DataCenter DataCenter;

typedef struct
{
    Account* items[ACCOUNT_LIST_MAX];
    uint16_t n;
} AccountList;

enum
{
    ACCOUNT_EVENT_NONE = 0,
    ACCOUNT_EVENT_PUB_PUBLISH,
    ACCOUNT_EVENT_SUB_PULL,
    ACCOUNT_EVENT_NOTIFY,
    ACCOUNT_EVENT_TIMER
};

enum
{
    ACCOUNT_RES_OK                  =  0,
    ACCOUNT_RES_UNKNOW              = -1,
    ACCOUNT_RES_SIZE_MISMATCH       = -2,
    ACCOUNT_RES_UNSUPPORTED_REQUEST = -3,
    ACCOUNT_RES_NO_CALLBACK         = -4,
    ACCOUNT_RES_NO_CACHE            = -5,
    ACCOUNT_RES_NO_COMMITED         = -6,
    ACCOUNT_RES_NOT_FOUND           = -7,
    ACCOUNT_RES_PARAM_ERROR         = -8
};

typedef int (*Account_Callback)(Account* a, int event, void* from, void* data, uint32_t size);

struct Account
{
    const char* ID;
    DataCenter* Center;
    void* UserData;

    AccountList publishers;
    AccountList subscribers;

    struct
    {
        Account_Callback eventCallback;
        lv_timer_t* timer;
        PingPongBuffer_t BufferManager;
        uint32_t BufferSize;
    } priv;
};

void        Account_Init(Account* a, const char* id, DataCenter* center, uint32_t bufSize, void* userData);
void        Account_Deinit(Account* a);
Account*    Account_Create(const char* id, void* center, uint32_t buf, void* user);
void        Account_Destroy(Account* a);
int         Account_Subscribe(Account* a, const char* name);
int         Account_Unsubscribe(Account* a, const char* name);
int         Account_Pull(Account* a, const char* name, void* buf, uint32_t size);
int         Account_Notify(Account* a, const char* name, const void* buf, uint32_t size);
bool        Account_Commit(Account* a, const void* data, uint32_t size);
int         Account_Publish(Account* a);
void        Account_SetCallback(Account* a, Account_Callback cb);
void        Account_SetTimerPeriod(Account* a, uint32_t period);
void        Account_SetTimerEnable(Account* a, bool en);
const char* Account_GetID(Account* a);
void*       Account_GetUser(Account* a);
size_t      Account_GetPublishersSize(Account* a);
size_t      Account_GetSubscribersSize(Account* a);

#ifdef __cplusplus
}
#endif

#endif

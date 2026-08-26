#ifndef __ACCOUNT_C_H
#define __ACCOUNT_C_H

#include <stdint.h>

#ifdef __cplusplus
class Account;
#else
typedef struct Account Account;
#endif

#ifdef __cplusplus
extern "C" {
#endif

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

Account*    Account_Create(const char* id, void* center, uint32_t buf, void* user);
void        Account_Destroy(Account* a);
int         Account_Subscribe(Account* a, const char* name);
int         Account_Pull(Account* a, const char* name, void* buf, uint32_t size);
int         Account_Notify(Account* a, const char* name, const void* buf, uint32_t size);
void        Account_SetCallback(Account* a, Account_Callback cb);
const char* Account_GetID(Account* a);
void*       Account_GetUser(Account* a);

#ifdef __cplusplus
}
#endif

#endif

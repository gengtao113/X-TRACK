#ifndef __DATA_CENTER_H
#define __DATA_CENTER_H

#include "Account.h"

#ifdef __cplusplus
extern "C" {
#endif

struct DataCenter
{
    const char* Name;
    Account AccountMain;
    AccountList AccountPool;
};

void     DataCenter_Init(DataCenter* dc, const char* name);
void     DataCenter_Deinit(DataCenter* dc);
bool     DataCenter_AddAccount(DataCenter* dc, Account* account);
bool     DataCenter_RemoveAccount(DataCenter* dc, Account* account);
bool     DataCenter_Remove(AccountList* list, Account* account);
Account* DataCenter_SearchAccount(DataCenter* dc, const char* id);
Account* DataCenter_Find(AccountList* list, const char* id);
size_t   DataCenter_GetAccountLen(DataCenter* dc);

#ifdef __cplusplus
}
#endif

#endif

#include "account_c.h"
#include "Account.h"
#include "DataCenter.h"

struct AccountC
{
    Account account;
    Account_Callback cb;

    AccountC(const char* id, DataCenter* center, uint32_t buf, void* user)
        : account(id, center, buf, user)
        , cb(nullptr)
    {
    }
};

static int AccountC_Trampoline(Account* a, Account::EventParam_t* param)
{
    AccountC* wrap = (AccountC*)a;
    if (wrap->cb == nullptr)
    {
        return Account::RES_NO_CALLBACK;
    }
    return wrap->cb(a, (int)param->event, param->tran, param->data_p, param->size);
}

Account* Account_Create(const char* id, void* center, uint32_t buf, void* user)
{
    AccountC* wrap = new AccountC(id, (DataCenter*)center, buf, user);
    return &wrap->account;
}

void Account_Destroy(Account* a)
{
    if (a == nullptr)
    {
        return;
    }
    delete (AccountC*)a;
}

int Account_Subscribe(Account* a, const char* name)
{
    if (a == nullptr || a->Subscribe(name) == nullptr)
    {
        return -1;
    }
    return 0;
}

int Account_Pull(Account* a, const char* name, void* buf, uint32_t size)
{
    if (a == nullptr)
    {
        return ACCOUNT_RES_PARAM_ERROR;
    }
    return a->Pull(name, buf, size);
}

int Account_Notify(Account* a, const char* name, const void* buf, uint32_t size)
{
    if (a == nullptr)
    {
        return ACCOUNT_RES_PARAM_ERROR;
    }
    return a->Notify(name, buf, size);
}

void Account_SetCallback(Account* a, Account_Callback cb)
{
    if (a == nullptr)
    {
        return;
    }
    AccountC* wrap = (AccountC*)a;
    wrap->cb = cb;
    a->SetEventCallback(AccountC_Trampoline);
}

const char* Account_GetID(Account* a)
{
    return a ? a->ID : nullptr;
}

void* Account_GetUser(Account* a)
{
    return a ? a->UserData : nullptr;
}

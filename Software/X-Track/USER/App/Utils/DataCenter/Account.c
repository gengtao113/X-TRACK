/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OF OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "Account.h"
#include "DataCenter.h"
#include "DataCenterLog.h"
#include <string.h>
#include <stdlib.h>

#define ACCOUNT_DISCARD_READ_DATA   1

static void Account_TimerCallback(lv_timer_t* timer);
static int  Account_PullFrom(Account* a, Account* pub, void* data_p, uint32_t size);
static int  Account_NotifyTo(Account* a, Account* pub, const void* data_p, uint32_t size);

void Account_Init(Account* a, const char* id, DataCenter* center, uint32_t bufSize, void* userData)
{
    memset(&a->priv, 0, sizeof(a->priv));
    memset(&a->publishers, 0, sizeof(a->publishers));
    memset(&a->subscribers, 0, sizeof(a->subscribers));

    a->ID = id;
    a->Center = center;
    a->UserData = userData;

    if (bufSize != 0)
    {
        uint8_t* buffer = (uint8_t*)lv_mem_alloc(bufSize * sizeof(uint8_t) * 2);

        if (!buffer)
        {
            DC_LOG_ERROR("Account[%s] buffer malloc failed!", a->ID);
            return;
        }

        memset(buffer, 0, bufSize * sizeof(uint8_t) * 2);

        PingPongBuffer_Init(&a->priv.BufferManager, buffer, buffer + bufSize);
        DC_LOG_INFO("Account[%s] cached %d x2 bytes", a->ID, bufSize);
        a->priv.BufferSize = bufSize;
    }

    DataCenter_AddAccount(a->Center, a);

    DC_LOG_INFO("Account[%s] created", a->ID);
}

void Account_Deinit(Account* a)
{
    DC_LOG_INFO("Account[%s] deleting...", a->ID);

    if (a->priv.BufferSize)
    {
        lv_mem_free(a->priv.BufferManager.buffer[0]);
    }

    if (a->priv.timer)
    {
        lv_timer_del(a->priv.timer);
        DC_LOG_INFO("Account[%s] task deleted", a->ID);
    }

    {
        uint16_t n = a->subscribers.n;
        Account* tmp[ACCOUNT_LIST_MAX];
        uint16_t i;
        memcpy(tmp, a->subscribers.items, n * sizeof(Account*));
        for (i = 0; i < n; i++)
        {
            Account_Unsubscribe(tmp[i], a->ID);
            DC_LOG_INFO("sub[%s] unsubscribed pub[%s]", tmp[i]->ID, a->ID);
        }
    }

    {
        uint16_t i;
        for (i = 0; i < a->publishers.n; i++)
        {
            Account* iter = a->publishers.items[i];
            DataCenter_Remove(&iter->subscribers, a);
            DC_LOG_INFO("pub[%s] removed sub[%s]", iter->ID, a->ID);
        }
    }

    DataCenter_RemoveAccount(a->Center, a);
    DC_LOG_INFO("Account[%s] deleted", a->ID);
}

Account* Account_Create(const char* id, void* center, uint32_t buf, void* user)
{
    Account* a = (Account*)malloc(sizeof(Account));
    if (a == NULL)
    {
        return NULL;
    }
    Account_Init(a, id, (DataCenter*)center, buf, user);
    return a;
}

void Account_Destroy(Account* a)
{
    if (a == NULL)
    {
        return;
    }
    Account_Deinit(a);
    free(a);
}

int Account_Subscribe(Account* a, const char* pubID)
{
    Account* pub;

    if (a == NULL || pubID == NULL)
    {
        return -1;
    }

    if (strcmp(pubID, a->ID) == 0)
    {
        DC_LOG_ERROR("Account[%s] try to subscribe to it itself", a->ID);
        return -1;
    }

    pub = DataCenter_Find(&a->publishers, pubID);
    if (pub != NULL)
    {
        DC_LOG_ERROR("Multi subscribe pub[%s]", pubID);
        return -1;
    }

    pub = DataCenter_SearchAccount(a->Center, pubID);
    if (pub == NULL)
    {
        DC_LOG_ERROR("pub[%s] was not found", pubID);
        return -1;
    }

    if (a->publishers.n >= ACCOUNT_LIST_MAX || pub->subscribers.n >= ACCOUNT_LIST_MAX)
    {
        DC_LOG_ERROR("Account[%s] subscribe list is full", a->ID);
        return -1;
    }
    a->publishers.items[a->publishers.n++] = pub;

    pub->subscribers.items[pub->subscribers.n++] = a;

    DC_LOG_INFO("sub[%s] subscribed pub[%s]", a->ID, pubID);

    return 0;
}

int Account_Unsubscribe(Account* a, const char* pubID)
{
    Account* pub;

    if (a == NULL)
    {
        return -1;
    }

    pub = DataCenter_Find(&a->publishers, pubID);
    if (pub == NULL)
    {
        DC_LOG_WARN("sub[%s] was not subscribe pub[%s]", a->ID, pubID);
        return -1;
    }

    DataCenter_Remove(&a->publishers, pub);
    DataCenter_Remove(&pub->subscribers, a);

    return 0;
}

bool Account_Commit(Account* a, const void* data_p, uint32_t size)
{
    void* wBuf;

    if (a == NULL)
    {
        return false;
    }

    if (!size || size != a->priv.BufferSize)
    {
        DC_LOG_ERROR("pub[%s] has not cache", a->ID);
        return false;
    }

    PingPongBuffer_GetWriteBuf(&a->priv.BufferManager, &wBuf);
    memcpy(wBuf, data_p, size);
    PingPongBuffer_SetWriteDone(&a->priv.BufferManager);

    DC_LOG_INFO("pub[%s] commit data(0x%p)[%d] >> data(0x%p)[%d] done",
                a->ID, data_p, size, wBuf, size);

    return true;
}

int Account_Publish(Account* a)
{
    int retval = ACCOUNT_RES_UNKNOW;
    void* rBuf;
    Account_Callback callback;
    uint16_t i;

    if (a == NULL)
    {
        return ACCOUNT_RES_PARAM_ERROR;
    }

    if (a->priv.BufferSize == 0)
    {
        DC_LOG_ERROR("pub[%s] has not cache", a->ID);
        return ACCOUNT_RES_NO_CACHE;
    }

    if (!PingPongBuffer_GetReadBuf(&a->priv.BufferManager, &rBuf))
    {
        DC_LOG_WARN("pub[%s] data was not commit", a->ID);
        return ACCOUNT_RES_NO_COMMITED;
    }

    for (i = 0; i < a->subscribers.n; i++)
    {
        Account* sub = a->subscribers.items[i];
        callback = sub->priv.eventCallback;

        DC_LOG_INFO("pub[%s] publish >> data(0x%p)[%d] >> sub[%s]...",
                    a->ID, rBuf, a->priv.BufferSize, sub->ID);

        if (callback != NULL)
        {
            int ret = callback(sub, ACCOUNT_EVENT_PUB_PUBLISH, a, rBuf, a->priv.BufferSize);

            DC_LOG_INFO("publish done: %d", ret);
            retval = ret;
        }
        else
        {
            DC_LOG_INFO("sub[%s] not register callback", sub->ID);
        }
    }

#if ACCOUNT_DISCARD_READ_DATA
    PingPongBuffer_SetReadDone(&a->priv.BufferManager);
#endif

    return retval;
}

int Account_Pull(Account* a, const char* pubID, void* data_p, uint32_t size)
{
    Account* pub;

    if (a == NULL)
    {
        return ACCOUNT_RES_PARAM_ERROR;
    }

    pub = DataCenter_Find(&a->publishers, pubID);
    if (pub == NULL)
    {
        DC_LOG_ERROR("sub[%s] was not subscribe pub[%s]", a->ID, pubID);
        return ACCOUNT_RES_NOT_FOUND;
    }
    return Account_PullFrom(a, pub, data_p, size);
}

static int Account_PullFrom(Account* a, Account* pub, void* data_p, uint32_t size)
{
    int retval = ACCOUNT_RES_UNKNOW;
    Account_Callback callback;

    if (pub == NULL)
    {
        return ACCOUNT_RES_NOT_FOUND;
    }

    DC_LOG_INFO("sub[%s] pull << data(0x%p)[%d] << pub[%s] ...",
                a->ID, data_p, size, pub->ID);

    callback = pub->priv.eventCallback;
    if (callback != NULL)
    {
        int ret = callback(pub, ACCOUNT_EVENT_SUB_PULL, a, data_p, size);

        DC_LOG_INFO("pull done: %d", ret);
        retval = ret;
    }
    else
    {
        DC_LOG_INFO("pub[%s] not registed pull callback, read commit cache...", pub->ID);

        if (pub->priv.BufferSize == size)
        {
            void* rBuf;
            if (PingPongBuffer_GetReadBuf(&pub->priv.BufferManager, &rBuf))
            {
                memcpy(data_p, rBuf, size);
#if ACCOUNT_DISCARD_READ_DATA
                PingPongBuffer_SetReadDone(&pub->priv.BufferManager);
#endif
                DC_LOG_INFO("read done");
                retval = 0;
            }
            else
            {
                DC_LOG_WARN("pub[%s] data was not commit!", pub->ID);
            }
        }
        else
        {
            DC_LOG_ERROR(
                "Data size pub[%s]:%d != sub[%s]:%d",
                pub->ID,
                pub->priv.BufferSize,
                a->ID,
                size
            );
        }
    }

    return retval;
}

int Account_Notify(Account* a, const char* pubID, const void* data_p, uint32_t size)
{
    Account* pub;

    if (a == NULL)
    {
        return ACCOUNT_RES_PARAM_ERROR;
    }

    pub = DataCenter_Find(&a->publishers, pubID);
    if (pub == NULL)
    {
        DC_LOG_ERROR("sub[%s] was not subscribe pub[%s]", a->ID, pubID);
        return ACCOUNT_RES_NOT_FOUND;
    }
    return Account_NotifyTo(a, pub, data_p, size);
}

static int Account_NotifyTo(Account* a, Account* pub, const void* data_p, uint32_t size)
{
    int retval = ACCOUNT_RES_UNKNOW;
    Account_Callback callback;

    if (pub == NULL)
    {
        return ACCOUNT_RES_NOT_FOUND;
    }

    DC_LOG_INFO("sub[%s] notify >> data(0x%p)[%d] >> pub[%s] ...",
                a->ID, data_p, size, pub->ID);

    callback = pub->priv.eventCallback;
    if (callback != NULL)
    {
        int ret = callback(pub, ACCOUNT_EVENT_NOTIFY, a, (void*)data_p, size);

        DC_LOG_INFO("send done: %d", ret);
        retval = ret;
    }
    else
    {
        DC_LOG_WARN("pub[%s] not register callback", pub->ID);
        retval = ACCOUNT_RES_NO_CALLBACK;
    }

    return retval;
}

void Account_SetCallback(Account* a, Account_Callback cb)
{
    if (a == NULL)
    {
        return;
    }
    a->priv.eventCallback = cb;
}

static void Account_TimerCallback(lv_timer_t* timer)
{
    Account* instance = (Account*)(timer->user_data);
    Account_Callback callback = instance->priv.eventCallback;
    if (callback)
    {
        callback(instance, ACCOUNT_EVENT_TIMER, instance, NULL, 0);
    }
}

void Account_SetTimerPeriod(Account* a, uint32_t period)
{
    if (a == NULL)
    {
        return;
    }

    if (a->priv.timer)
    {
        lv_timer_del(a->priv.timer);
        a->priv.timer = NULL;
    }

    if (period == 0)
    {
        return;
    }

    a->priv.timer = lv_timer_create(
                        Account_TimerCallback,
                        period,
                        a
                    );
}

void Account_SetTimerEnable(Account* a, bool en)
{
    lv_timer_t* timer;

    if (a == NULL)
    {
        return;
    }

    timer = a->priv.timer;
    if (timer == NULL)
    {
        return;
    }

    en ? lv_timer_resume(timer) : lv_timer_pause(timer);
}

const char* Account_GetID(Account* a)
{
    return a ? a->ID : NULL;
}

void* Account_GetUser(Account* a)
{
    return a ? a->UserData : NULL;
}

size_t Account_GetPublishersSize(Account* a)
{
    return a ? a->publishers.n : 0;
}

size_t Account_GetSubscribersSize(Account* a)
{
    return a ? a->subscribers.n : 0;
}

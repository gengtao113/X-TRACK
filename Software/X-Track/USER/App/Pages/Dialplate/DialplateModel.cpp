#include "DialplateModel.h"

using namespace Page;

/**
  * @brief  初始化表盘 Model：在 DataCenter 上开账号并订阅节点
  * @note   C 对应 DialplateModel_Init(&model)。
  *         new Account(..., 0, this)：缓冲长度 0（本页不 Publish），
  *         UserData = this，供 static onEvent 取回对象。
  *         Subscribe 只是声明要跟谁说话；真正收数的是 SportStatus 的 Publish。
  * @retval None
  */
void DialplateModel::Init()
{
    /* "DialplateModel" : 总线上的账号名，必须唯一
     * Center()         : 全局 DataCenter
     * 0                : 发送缓冲长度，本页不 Publish
     * this             : UserData，static onEvent 里转回 DialplateModel* */
    account = new Account("DialplateModel", DataProc::Center(), 0, this);
    account->Subscribe("SportStatus");   ///< 收运动数据 Publish，拷进 sportStatusInfo
    account->Subscribe("Recorder");      ///< 为 Notify 录轨命令建立订阅关系
    account->Subscribe("StatusBar");     ///< 为 Notify 状态栏样式/REC 标签建立订阅关系
    account->Subscribe("GPS");           ///< 为 Pull 卫星数（GetGPSReady）建立订阅关系
    account->Subscribe("MusicPlayer");   ///< 为 Notify 提示音建立订阅关系
    account->SetEventCallback(onEvent);  ///< 挂 Publish 回调；static，无 this
}

/**
  * @brief  反初始化：注销账号
  * @note   C 对应 DialplateModel_Deinit()。delete account 会 Unsubscribe。
  *         必须与 Init 成对，页面 UNLOAD 时调用，否则总线里残留账号。
  * @retval None
  */
void DialplateModel::Deinit()
{
    if (account)
    {
        delete account;
        account = nullptr;
    }
}

/**
  * @brief  查询 GPS 是否可用
  * @note   向 "GPS" 节点 Pull 一份 GPS_Info_t，卫星数 > 0 视为就绪。
  *         失败（节点不在、长度不对）也返回 false。不直接读 UART。
  * @retval true  GPS 有卫星；false 不可用
  */
bool DialplateModel::GetGPSReady()
{
    HAL::GPS_Info_t gps;
    if(account->Pull("GPS", &gps, sizeof(gps)) != Account::RES_OK)
    {
        return false;
    }
    return (gps.satellites > 0);
}

/**
  * @brief  总线事件回调：只处理 SportStatus 的 Publish
  * @param  account  本页账号（UserData 里是 DialplateModel*）
  * @param  param    事件参数：类型、发送方、数据指针和长度
  * @note   static 成员 = C 普通函数，没有 this。
  *         非 PUBLISH 返回 UNSUPPORTED；名字或 size 不对返回 PARAM_ERROR。
  *         匹配则 memcpy 到 sportStatusInfo，供 GetSpeed 等直接读。
  * @retval Account::RES_OK 或错误码
  */
int DialplateModel::onEvent(Account* account, Account::EventParam_t* param)
{
    if (param->event != Account::EVENT_PUB_PUBLISH)
    {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (strcmp(param->tran->ID, "SportStatus") != 0
            || param->size != sizeof(HAL::SportStatus_Info_t))
    {
        return Account::RES_PARAM_ERROR;
    }

    DialplateModel* instance = (DialplateModel*)account->UserData;
    memcpy(&(instance->sportStatusInfo), param->data_p, param->size);

    return Account::RES_OK;
}

/**
  * @brief  发送录轨命令，并同步状态栏 REC 标签
  * @param  cmd  REC_START / PAUSE / CONTINUE / STOP / READY_STOP
  * @note   REC_READY_STOP 只改状态栏显示 "STOP"，不 Notify Recorder（还没真正停文件）。
  *         其余命令先 Notify "Recorder"（time=1000ms 采样），再 Notify "StatusBar"。
  *         真正写 GPX 在 Recorder 节点，本函数不碰文件系统。
  * @retval None
  */
void DialplateModel::RecorderCommand(RecCmd_t cmd)
{
    if (cmd != REC_READY_STOP)
    {
        DataProc::Recorder_Info_t recInfo;
        DATA_PROC_INIT_STRUCT(recInfo);
        recInfo.cmd = (DataProc::Recorder_Cmd_t)cmd;
        recInfo.time = 1000;
        account->Notify("Recorder", &recInfo, sizeof(recInfo));
    }

    DataProc::StatusBar_Info_t statInfo;
    DATA_PROC_INIT_STRUCT(statInfo);
    statInfo.cmd = DataProc::STATUS_BAR_CMD_SET_LABEL_REC;

    switch (cmd)
    {
    case REC_START:
    case REC_CONTINUE:
        statInfo.param.labelRec.show = true;
        statInfo.param.labelRec.str = "REC";
        break;
    case REC_PAUSE:
        statInfo.param.labelRec.show = true;
        statInfo.param.labelRec.str = "PAUSE";
        break;  
    case REC_READY_STOP:
        statInfo.param.labelRec.show = true;
        statInfo.param.labelRec.str = "STOP";
        break;
    case REC_STOP:
        statInfo.param.labelRec.show = false;
        break;
    default:
        break;
    }

    account->Notify("StatusBar", &statInfo, sizeof(statInfo));
}

/**
  * @brief  播放提示音
  * @param  music  曲目名（如 "Connect" / "Error"），由 MusicPlayer 节点解释
  * @note   Notify "MusicPlayer"。表盘自己不调蜂鸣器 HAL。
  * @retval None
  */
void DialplateModel::PlayMusic(const char* music)
{
    DataProc::MusicPlayer_Info_t info;
    DATA_PROC_INIT_STRUCT(info);

    info.music = music;
    account->Notify("MusicPlayer", &info, sizeof(info));
}

/**
  * @brief  设置状态栏样式
  * @param  style  例如 STATUS_BAR_STYLE_TRANSP（表盘半透明状态栏）
  * @note   Notify "StatusBar"，命令 SET_STYLE。
  * @retval None
  */
void DialplateModel::SetStatusBarStyle(DataProc::StatusBar_Style_t style)
{
    DataProc::StatusBar_Info_t info;
    DATA_PROC_INIT_STRUCT(info);

    info.cmd = DataProc::STATUS_BAR_CMD_SET_STYLE;
    info.param.style = style;

    account->Notify("StatusBar", &info, sizeof(info));
}

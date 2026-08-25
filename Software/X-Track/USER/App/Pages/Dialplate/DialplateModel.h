#ifndef __DIALPLATE_MODEL_H
#define __DIALPLATE_MODEL_H

#include "Common/DataProc/DataProc.h"

/* namespace = 名字前缀，避免和别的 DialplateModel 重名。
 * 调用时写成 Page::DialplateModel，可当成 C 里的模块名忽略。 */
namespace Page
{

/* class ≈ C 的 struct + 一组操作函数。
 * 成员函数调用 Model.Init() 时，编译器会自动传入 this（即 Model 自己的地址），
 * 等价于 C：DialplateModel_Init(&model)。 */
class DialplateModel
{
public:  /* public：外面可以访问，类似 C 头文件里对外声明的函数/字段 */

    /* 录轨命令。数值和 DataProc 里 Recorder 节点的命令对齐。
     * REC_READY_STOP 只改状态栏显示 STOP，还不真正停录。 */
    typedef enum
    {
        REC_START    = DataProc::RECORDER_CMD_START,
        REC_PAUSE    = DataProc::RECORDER_CMD_PAUSE,
        REC_CONTINUE = DataProc::RECORDER_CMD_CONTINUE,
        REC_STOP     = DataProc::RECORDER_CMD_STOP,
        REC_READY_STOP
    } RecCmd_t;

public:
    /* 运动数据缓存。SportStatus 节点 Publish 后，onEvent 里 memcpy 进来。
     * Presenter 读速度/里程，直接看这块，不必再问硬件。 */
    HAL::SportStatus_Info_t sportStatusInfo;

public:
    void Init();    /* 向 DataCenter 注册账号并 Subscribe，C：xxx_init() */
    void Deinit();  /* 注销账号，C：xxx_deinit() */

    /* 向名为 "GPS" 的节点 Pull 一份 GPS_Info_t，卫星数 > 0 则认为可用 */
    bool GetGPSReady();

    /* 写在头文件里的函数 = inline：调用处直接展开，没有单独 .cpp。
     * 就是 return m->sportStatusInfo.speedKph; */
    float GetSpeed()
    {
        return sportStatusInfo.speedKph;
    }

    float GetAvgSpeed()
    {
        return sportStatusInfo.speedAvgKph;
    }

    void RecorderCommand(RecCmd_t cmd);   /* Notify "Recorder" / "StatusBar" */
    void PlayMusic(const char* music);    /* Notify "MusicPlayer" */
    void SetStatusBarStyle(DataProc::StatusBar_Style_t style);

private:  /* private：只有本 class 内部能用，头文件里给别人看但不能直接改 */

    /* 本页在 DataCenter 上的账号。Subscribe / Pull / Notify 都通过它。
     * C 里就是一个句柄指针：Account_t *account; */
    Account* account;

private:
    /* static 成员函数 = 普通 C 函数，没有隐藏的 this。
     * 必须做成这种形式，才能当作 Account 的 C 回调挂上去。
     * 真正的 Model 指针放在 account->UserData 里，回调里再转回来。 */
    static int onEvent(Account* account, Account::EventParam_t* param);
};

}

#endif


#include "DialplateModel.h"

using namespace Page;

void DialplateModel::Init()
{
    account = new Account("DialplateModel", DataProc::Center(), 0, this);//传入this指针，以便回调函数中使用
    account->Subscribe("SportStatus");
    account->Subscribe("Recorder");
    account->Subscribe("StatusBar");
    account->Subscribe("GPS");
    account->Subscribe("MusicPlayer");
    account->SetEventCallback(onEvent);//为account设置callback
}

void DialplateModel::Deinit()
{
    if (account)
    {
        delete account;
        account = nullptr;
    }
}

bool DialplateModel::GetGPSReady()
{
    HAL::GPS_Info_t gps;
    if(account->Pull("GPS", &gps, sizeof(gps)) != Account::RES_OK)
    {
        return false;
    }
    return (gps.satellites > 0);//卫星数量大于0表示GPS ready
}

//处理SportStatus的Publish事件
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
    memcpy(&(instance->sportStatusInfo), param->data_p, param->size);//copy data

    return Account::RES_OK;
}

/**
 * @brief 向Recorder account发送cmd, 而Recorder account又会向TrackFilter account发送cmd
 *        在StatusBar显示REC/PAUSE/STOP
 * @param cmd
 */
void DialplateModel::RecorderCommand(RecCmd_t cmd)
{
    //向Recorder发送cmd，REC_READY_STOP不用发送，因为Recorder没有这个状态
    if (cmd != REC_READY_STOP)
    {
        DataProc::Recorder_Info_t recInfo;
        DATA_PROC_INIT_STRUCT(recInfo);
        recInfo.cmd = (DataProc::Recorder_Cmd_t)cmd;
        recInfo.time = 1000;
        //向Recorder account发送cmd, 而Recorder account又会向TrackFilter account发送cmd
        account->Notify("Recorder", &recInfo, sizeof(recInfo));
    }

    //在StatusBar显示REC/PAUSE/STOP
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

void DialplateModel::PlayMusic(const char* music)
{
    DataProc::MusicPlayer_Info_t info;
    DATA_PROC_INIT_STRUCT(info);

    info.music = music;
    account->Notify("MusicPlayer", &info, sizeof(info));
}

//设置StatusBar的style
void DialplateModel::SetStatusBarStyle(DataProc::StatusBar_Style_t style)
{
    DataProc::StatusBar_Info_t info;
    DATA_PROC_INIT_STRUCT(info);

    info.cmd = DataProc::STATUS_BAR_CMD_SET_STYLE;
    info.param.style = style;

    account->Notify("StatusBar", &info, sizeof(info));
}

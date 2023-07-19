#include "DataProc.h"
#include "../UTILS/Filters/Filters.h"
#include "power.h"


static void onTimer(Account* account)
{
    static bool lastStatus = false;

    //定时器检查电池充电状态 电池充电状态改变时通知音乐播放器
    HAL::Power_Info_t power;
    Power_GetInfo(&power);
    if (power.isCharging != lastStatus)
    {
        DataProc::MusicPlayer_Info_t info;
        DATA_PROC_INIT_STRUCT(info);
        info.music = power.isCharging ? "BattChargeStart" : "BattChargeEnd";
        account->Notify("MusicPlayer", &info, sizeof(info));

        lastStatus = power.isCharging;
    }
}

static int onEvent(Account* account, Account::EventParam_t* param)
{
    static Filter::Hysteresis<int16_t>      battUsageHysFilter(2);
    static Filter::MedianQueue<int16_t, 10> battUsageMqFilter;

    if (param->event == Account::EVENT_TIMER)
    {
        onTimer(account);
        return Account::RES_OK;
    }

    if (param->event != Account::EVENT_SUB_PULL)
    {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (param->size != sizeof(HAL::Power_Info_t))
    {
        return Account::RES_SIZE_MISMATCH;
    }

    HAL::Power_Info_t powerInfo;
    Power_GetInfo(&powerInfo);

    int16_t usage = powerInfo.usage;
    usage = battUsageHysFilter.GetNext(usage);
    usage = battUsageMqFilter.GetNext(usage);
    powerInfo.usage = (uint8_t)usage;

    memcpy(param->data_p, &powerInfo, param->size);

    return Account::RES_OK;
}

DATA_PROC_INIT_DEF(Power)
{
    account->Subscribe("MusicPlayer");
    account->SetEventCallback(onEvent);
    account->SetTimerPeriod(500);
}

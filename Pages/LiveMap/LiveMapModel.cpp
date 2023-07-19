#include "LiveMapModel.h"
#include "Config.h"
#include "PointContainer/PointContainer.h"

using namespace Page;

LiveMapModel::LiveMapModel()
{

}

void LiveMapModel::Init()
{
    account = new Account("LiveMapModel", DataProc::Center(), 0, this);
    account->Subscribe("GPS");
    account->Subscribe("SportStatus");
    account->Subscribe("TrackFilter");
    account->Subscribe("SysConfig");
    account->Subscribe("StatusBar");
    account->SetEventCallback(onEvent);
}

void LiveMapModel::Deinit()
{
    if (account)
    {
        delete account;
        account = nullptr;
    }
}

void LiveMapModel::GetGPS_Info(HAL::GPS_Info_t* info)
{
    memset(info, 0, sizeof(HAL::GPS_Info_t));
    if(account->Pull("GPS", info, sizeof(HAL::GPS_Info_t)) != Account::RES_OK)
    {
        return;
    }

    /* Use default location */
    if (!info->isVaild)
    {
        DataProc::SysConfig_Info_t sysConfig;
        if(account->Pull("SysConfig", &sysConfig, sizeof(sysConfig)) == Account::RES_OK)
        {
            info->longitude = sysConfig.longitude;
            info->latitude = sysConfig.latitude;
        }
    }
}

void LiveMapModel::GetArrowTheme(char* buf, uint32_t size)
{
    DataProc::SysConfig_Info_t sysConfig;
    if(account->Pull("SysConfig", &sysConfig, sizeof(sysConfig)) != Account::RES_OK)
    {
        buf[0] = '\0';
        return;
    }
    strncpy(buf, sysConfig.arrowTheme, size);
    buf[size - 1] = '\0';
}

/**
 * @brief get TrackFilter服务的状态
 * 
 * @return isActive 
 */
bool LiveMapModel::GetTrackFilterActive()
{
    DataProc::TrackFilter_Info_t info;
    if(account->Pull("TrackFilter", &info, sizeof(info)) != Account::RES_OK)
    {
        return false;
    }

    return info.isActive;
}
/**
 * @brief 接受SportStatus account pulish的 data
 * @param account
 * @param param
 * @return
 */
int LiveMapModel::onEvent(Account* account, Account::EventParam_t* param)
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
    //copy SportStatus account pulish的data to sportStatusInfo
    LiveMapModel* instance = (LiveMapModel*)account->UserData;
    memcpy(&(instance->sportStatusInfo), param->data_p, param->size);

    return Account::RES_OK;
}
/**
 * @brief 将TrackFilter服务中已经保存的点，更改level后push point to trackPointFilter，然后会调用callback处理
 * @param callback
 * @param userData
 */
void LiveMapModel::TrackReload(TrackPointFilter::Callback_t callback, void* userData)
{
    DataProc::TrackFilter_Info_t info;
    // Get TrackFilter Info
    if(account->Pull("TrackFilter", &info, sizeof(info)) != Account::RES_OK)
    {
        return;
    }

    if (!info.isActive || info.pointCont == nullptr)
    {
        return;
    }

    //pointContainer  准备Get point
    PointContainer* pointContainer = (PointContainer*)info.pointCont;
    pointContainer->ResetGetIndex();

    //TrackPointFilter Init for ptFilter
    TrackPointFilter ptFilter;
    ptFilter.SetOffsetThreshold(CONFIG_TRACK_FILTER_OFFSET_THRESHOLD);
    ptFilter.SetOutputPointCallback(callback);
    ptFilter.SetSecondFilterModeEnable(true);
    ptFilter.userData = userData;// 传递userData

    //Get point from pointContainer
    int32_t pointX, pointY;
    while (pointContainer->GetPoint(&pointX, &pointY))
    {
        //将地图上一个点的位置从一个级别转换到另一个级别。
        int32_t mapX, mapY;
        mapConv.ConvertMapLevelPos(
            &mapX, &mapY,
            pointX, pointY,
            info.level
        );
        //push point to  TrackPointFilter ,让callback处理
        ptFilter.PushPoint(mapX, mapY);
    }
    ptFilter.PushEnd();
}

void LiveMapModel::SetStatusBarStyle(DataProc::StatusBar_Style_t style)
{
    DataProc::StatusBar_Info_t info;
    DATA_PROC_INIT_STRUCT(info);

    info.cmd = DataProc::STATUS_BAR_CMD_SET_STYLE;
    info.param.style = style;

    account->Notify("StatusBar", &info, sizeof(info));
}

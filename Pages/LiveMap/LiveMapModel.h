#ifndef __LIVEMAP_MODEL_H
#define __LIVEMAP_MODEL_H

#include "../../lvgl/lvgl.h"
#include "MapConv.h"
#include "TileConv/TileConv.h"
#include "TrackFilter/TrackFilter.h"
#include "DataProc.h"
#include <vector>
#include "key.h"

namespace Page
{

class LiveMapModel
{
public:
    LiveMapModel();
    ~LiveMapModel() {}
    void Init();
    void Deinit();
    void GetGPS_Info(HAL::GPS_Info_t* info);
    void GetArrowTheme(char* buf, uint32_t size);
    bool GetTrackFilterActive();
    void TrackReload(TrackPointFilter::Callback_t callback, void* userData);
    void SetStatusBarStyle(DataProc::StatusBar_Style_t style);

    void SetKeySliderMode(bool mode){
        if(mode)
            SetKeyUserMode(MODE_SLIDER);
        else
            SetKeyUserMode(MODE_GROUP);
    }

public:
    MapConv mapConv;
    TileConv tileConv;
    TrackPointFilter pointFilter;//面会当前focusPoint 经过的TrackPoint
    TrackLineFilter lineFilter; //用来过滤TrackFilter服务中记录下来的TrackPoint,只描绘pointTileCont中的Track
    HAL::SportStatus_Info_t sportStatusInfo={0};

private:
    Account* account;

private:
    static int onEvent(Account* account, Account::EventParam_t* param);

private:
};

}

#endif

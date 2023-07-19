#include "DataProc.h"
#include "Config.h"
#include "gps.h"

typedef enum {
    GPS_STATUS_DISCONNECT,
    GPS_STATUS_UNSTABLE,
    GPS_STATUS_CONNECT,
} GPS_Status_t;

static void onTimer(Account *account) {
    HAL::GPS_Info_t gpsInfo;
    GPS_GetInfo(&gpsInfo);

    int satellites = gpsInfo.satellites;

    static GPS_Status_t nowStatus = GPS_STATUS_DISCONNECT;
    static GPS_Status_t lastStatus = GPS_STATUS_DISCONNECT;

    if (satellites > 7) {
        nowStatus = GPS_STATUS_CONNECT;
    } else if (satellites < 5 && satellites >= 3) {
        nowStatus = GPS_STATUS_UNSTABLE;
    } else if (satellites == 0) {
        nowStatus = GPS_STATUS_DISCONNECT;
    }
    //状态改变时候音效
    if (nowStatus != lastStatus) {
        const char *music[] =
                {
                        "Disconnect",
                        "UnstableConnect",
                        "Connect"
                };

        DataProc::MusicPlayer_Info_t info;
        DATA_PROC_INIT_STRUCT(info);
        info.music = music[nowStatus];
        account->Notify("MusicPlayer", &info, sizeof(info));
        lastStatus = nowStatus;
    }
    //卫星数量大于3个才上传
    if (satellites >= 3) {
        account->Commit(&gpsInfo, sizeof(gpsInfo));
        account->Publish();
    }
}

static int onEvent(Account *account, Account::EventParam_t *param) {
    if (param->event == Account::EVENT_TIMER) {
        onTimer(account);
        return Account::RES_OK;
    }

    if (param->event != Account::EVENT_SUB_PULL) {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (param->size != sizeof(HAL::GPS_Info_t)) {
        return Account::RES_SIZE_MISMATCH;
    }
    //获取GPS位置信息与GPS时间信息
    GPS_GetInfo((HAL::GPS_Info_t *) param->data_p);

    return Account::RES_OK;
}

DATA_PROC_INIT_DEF(GPS) {
    account->Subscribe("MusicPlayer");

    account->SetEventCallback(onEvent);
    account->SetTimerPeriod(CONFIG_GPS_REFR_PERIOD);//定时事件
}

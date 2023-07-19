#include "DataProc.h"
#include "../TIME/Utils/Time.h"
#include <stdio.h>

using namespace DataProc;

static int onEvent(Account *account, Account::EventParam_t *param) {
    if (param->event != Account::EVENT_SUB_PULL) {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (param->size != sizeof(HAL::Clock_Info_t)) {
        return Account::RES_SIZE_MISMATCH;
    }

    HAL::GPS_Info_t gps;
    if (account->Pull("GPS", &gps, sizeof(gps)) != Account::RES_OK) {
        return Account::RES_UNKNOW;
    }

    DataProc::SysConfig_Info_t sysCfg;
    if (account->Pull("SysConfig", &sysCfg, sizeof(sysCfg)) != Account::RES_OK) {
        return Account::RES_UNKNOW;
    }
    //使用gps的时间设置timelib的时间
    setTime(
            gps.clock.hour,
            gps.clock.minute,
            gps.clock.second,
            gps.clock.day,
            gps.clock.month,
            gps.clock.year
    );
    //根据时区调整时间
    adjustTime(sysCfg.timeZone * SECS_PER_HOUR);

    //更新Clock_Info_t时间
    HAL::Clock_Info_t *info = (HAL::Clock_Info_t *) param->data_p;
    info->year = year();
    info->month = month();
    info->day = day();
    info->hour = hour();
    info->minute = minute();
    info->second = second();

    return Account::RES_OK;
}

DATA_PROC_INIT_DEF(TzConv) {
    account->Subscribe("GPS");
    account->Subscribe("SysConfig");
    account->SetEventCallback(onEvent);
}

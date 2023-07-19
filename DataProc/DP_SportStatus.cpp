#include "DataProc.h"
#include "Config.h"
#include "HAL_Def.h"
#include "gps.h"

#define CALORIC_CORFFICIENT 0.5f

using namespace DataProc;

static HAL::SportStatus_Info_t sportStatus;

//根据两次GPS获取两点间距离
static double SportStatus_GetDistanceOffset(HAL::GPS_Info_t *gpsInfo) {
    static bool isFirst = true;
    static double preLongitude;
    static double preLatitude;

    double offset = 0.0f;

    if (!isFirst) {
        offset = GPS_GetDistanceOffset(gpsInfo, preLongitude, preLatitude);
    } else {
        isFirst = false;
    }

    preLongitude = gpsInfo->longitude;
    preLatitude = gpsInfo->latitude;

    return offset;
}

static void onTimer(Account *account) {
    HAL::GPS_Info_t gpsInfo;
    if (account->Pull("GPS", &gpsInfo, sizeof(gpsInfo)) != Account::RES_OK) {
        return;
    }

    uint32_t timeElaps = DataProc::GetTickElaps(sportStatus.lastTick);//ms

    float speedKph = 0.0f;
    //信号不好
    bool isSignalInterruption = (gpsInfo.isVaild && (gpsInfo.satellites == 0));

    //卫星数大于等于3个才更新speedKph，速度大于1km/h才有效
    if (gpsInfo.satellites >= 3) {
        float spd = gpsInfo.speed;
        speedKph = spd > 1 ? spd : 0;
    }

    //速度大于0或者信号中断 需要更新数据
    if (speedKph > 0.0f || isSignalInterruption) {
        sportStatus.singleTime += timeElaps;
        sportStatus.totalTime += timeElaps;

        if (speedKph > 0.0f) {
            //计算距离
            float dist = (float) SportStatus_GetDistanceOffset(&gpsInfo);//meter

            sportStatus.singleDistance += dist;
            sportStatus.totalDistance += dist;

            //计算平均速度
            float meterPerSec = sportStatus.singleDistance * 1000 / sportStatus.singleTime;
            sportStatus.speedAvgKph = meterPerSec * 3.6f;

            //记录最大速度
            if (speedKph > sportStatus.speedMaxKph) {
                sportStatus.speedMaxKph = speedKph;
            }
            //卡路里消耗 = 骑行公里数 x 身体重量（千克） x 消耗因子
            float calorie = speedKph * sportStatus.weight * CALORIC_CORFFICIENT * timeElaps / 1000 / 3600;
            sportStatus.singleCalorie += calorie;
        }
    }

    sportStatus.speedKph = speedKph;

    sportStatus.lastTick = DataProc::GetTick();//
    account->Commit(&sportStatus, sizeof(sportStatus));
    account->Publish();//想订阅者发布数据
}

static int onEvent(Account *account, Account::EventParam_t *param) {
    if (param->event == Account::EVENT_TIMER) {
        onTimer(account);
        return Account::RES_OK;
    }

    if (param->event != Account::EVENT_SUB_PULL) {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (param->size != sizeof(sportStatus)) {
        return Account::RES_SIZE_MISMATCH;
    }
    //pull数据SportStatus_Info_t
    memcpy(param->data_p, &sportStatus, param->size);//
    return Account::RES_OK;
}

DATA_PROC_INIT_DEF(SportStatus) {
    memset(&sportStatus, 0, sizeof(sportStatus));
    sportStatus.weight = CONFIG_WEIGHT_DEFAULT;

    account->Subscribe("GPS");
    account->Subscribe("Storage");

    //add to storageservice  注意这里使用的是指针 后续会被CONFIG_SYSTEM_SAVE_FILE_PATH覆盖
    STORAGE_VALUE_REG(account, sportStatus,totalDistance, STORAGE_TYPE_FLOAT);
    STORAGE_VALUE_REG(account, sportStatus,totalTimeUINT32[0], STORAGE_TYPE_INT);
    STORAGE_VALUE_REG(account, sportStatus,totalTimeUINT32[1], STORAGE_TYPE_INT);
    STORAGE_VALUE_REG(account, sportStatus,speedMaxKph, STORAGE_TYPE_FLOAT);
    STORAGE_VALUE_REG(account, sportStatus,weight, STORAGE_TYPE_FLOAT);

    sportStatus.lastTick = DataProc::GetTick();

    account->SetEventCallback(onEvent);
    account->SetTimerPeriod(500);//定时事件
}

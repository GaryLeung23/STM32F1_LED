#include "power.h"

#define millis() HAL_GetTick()

#define BATT_ADC                    &hadc1
//#define BATT_MIN_VOLTAGE            3300
//#define BATT_MAX_VOLTAGE            4200
//#define BATT_FULL_CHARGE_VOLTAGE    4100

#define BATT_MIN_VOLTAGE            700
#define BATT_MAX_VOLTAGE            3300
#define BATT_FULL_CHARGE_VOLTAGE    3250

#define BATT_CHG_DET_STATUS         (HAL_GPIO_ReadPin(BAT_CHG_DET_GPIO_Port,BAT_CHG_DET_Pin) == GPIO_PIN_RESET)


struct {
    uint32_t LastHandleTime;//Power_HandleTimeUpdate()执行时刻 unit:ms
    uint16_t AutoLowPowerTimeout;//自动关机时间 unit:s
    bool AutoLowPowerEnable;//自动关机使能标志
    bool ShutdownReq;//关机请求
    uint16_t ADCValue;
    HAL::Power_CallbackFunction_t EventCallback;
} Power;


static void Power_ADC_Init(ADC_HandleTypeDef *ADCx) {
    //TODO

}

static uint16_t Power_ADC_GetValue() {
    uint16_t retval = 0;
    if (__HAL_ADC_GET_FLAG(&hadc1, ADC_FLAG_EOC)) {
        retval = HAL_ADC_GetValue(&hadc1);
    }
    return retval;
}


static void Power_ADC_Update() {
    static bool isStartConv = false;

    if (!isStartConv) {
        HAL_ADC_Start(BATT_ADC);
        isStartConv = true;
    } else {
        Power.ADCValue = Power_ADC_GetValue();
        isStartConv = false;
    }
}

void Power_HandleTimeUpdate() {
    Power.LastHandleTime = millis();
}

void Power_SetAutoLowPowerTimeout(uint16_t sec) {
    Power.AutoLowPowerTimeout = sec;
}

uint16_t Power_GetAutoLowPowerTimeout() {
    return Power.AutoLowPowerTimeout;
}

void Power_SetAutoLowPowerEnable(bool en) {
    Power.AutoLowPowerEnable = en;
    Power_HandleTimeUpdate();
}

void Power_Init() {
    memset(&Power, 0, sizeof(Power));
    Power.AutoLowPowerTimeout = 60;

    printf("Power: ON");

    Power_ADC_Init(BATT_ADC);

    Power_SetAutoLowPowerEnable(false);//这里的低电量关机功能没有启用
}

//函数只执行一次  只是设置标志
void Power_Shutdown() {
    CM_EXECUTE_ONCE(Power.ShutdownReq = true);
}

//power信息更新 定时运行
void Power_Update() {
    CM_EXECUTE_INTERVAL(Power_ADC_Update(), 1000);//1s

    if (!Power.AutoLowPowerEnable)
        return;

    if (Power.AutoLowPowerTimeout == 0)
        return;

    if (millis() - Power.LastHandleTime >= (Power.AutoLowPowerTimeout * 1000)) {
        Power_Shutdown();
    }
}

//关机事件监测   定时运行
void Power_EventMonitor() {
    if (Power.ShutdownReq) {
        if (Power.EventCallback) {
            Power.EventCallback();
        }
        //TODO  关机

        printf("Power: OFF");
        Power.ShutdownReq = false;
    }
}

void Power_GetInfo(HAL::Power_Info_t *info) {
    int voltage = CM_VALUE_MAP(
            Power.ADCValue,
            0, 4095,
            0, 3300
    );

//    voltage *= 2;    //跟硬件相关

    CM_VALUE_LIMIT(voltage, BATT_MIN_VOLTAGE, BATT_MAX_VOLTAGE);

    int usage = CM_VALUE_MAP(
            voltage,
            BATT_MIN_VOLTAGE, BATT_FULL_CHARGE_VOLTAGE,
            0, 100
    );

    CM_VALUE_LIMIT(usage, 0, 100);

    info->usage = usage;
    info->isCharging = BATT_CHG_DET_STATUS;//
    info->voltage = voltage;
}

//关机前需要做的操作  callback
void Power_SetEventCallback(HAL::Power_CallbackFunction_t callback) {
    Power.EventCallback = callback;
}

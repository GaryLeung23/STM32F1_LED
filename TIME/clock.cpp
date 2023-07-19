#include "clock.h"


void Clock_SetInfo(const HAL::Clock_Info_t *info) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    sTime.Hours = info->hour;
    sTime.Minutes = info->minute;
    sTime.Seconds = info->second;

    sDate.Year = info->year - 2000;
    sDate.Month = info->month;
    sDate.Date = info->day;
    sDate.WeekDay = info->week;


    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}


void Clock_GetInfo(HAL::Clock_Info_t *info) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    info->day = sDate.Date;
    info->month = sDate.Month;
    info->year = sDate.Year + 2000;
    info->week = sDate.WeekDay;

    info->hour = sTime.Hours;
    info->minute = sTime.Minutes;
    info->second = sTime.Seconds;

    info->millisecond = 0;

}
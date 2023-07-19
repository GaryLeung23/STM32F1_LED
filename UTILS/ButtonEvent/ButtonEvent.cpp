#include "ButtonEvent.h"

#ifdef ARDUINO
#  include "Arduino.h"
#  define GET_TICK() millis()
#else
#include "stm32f1xx_hal.h"
#include "string.h"
#define GET_TICK() HAL_GetTick()

#endif

#ifndef GET_TICK
#  error "Please define the GET_TICK() function to get the system time "
#endif

#ifndef UINT32_MAX
#  define UINT32_MAX  4294967295u
#endif

/**
  * @brief  按键事件构造函数
  * @param  longPressTime: 按键长按触发超时设置
  * @param  longPressTimeRepeat: 长按重复触发时间
  * @param  doubleClickTime: 双击间隔时间
  * @retval 无
  */
ButtonEvent::ButtonEvent(
    uint16_t longPressTime,
    uint16_t longPressTimeRepeat,
    uint16_t doubleClickTime
)
{
    memset(&priv, 0, sizeof(priv));

    priv.longPressTimeCfg = longPressTime;
    priv.longPressRepeatTimeCfg = longPressTimeRepeat;
    priv.doubleClickTimeCfg = doubleClickTime;

    priv.lastLongPressTime = priv.lastClickTime = priv.lastPressTime = 0;
    priv.isLongPressed = false;
    priv.nowState = STATE_NO_PRESS;

    priv.eventCallback = NULL;
}

/**
  * @brief  获取与上次时间的时间差(带uint32溢出识别)
  * @param  prevTick: 上的时间戳
  * @retval 时间差
  */
uint32_t ButtonEvent::GetTickElaps(uint32_t prevTick)
{
    uint32_t actTime = GET_TICK();
		uint32_t ElapsTime;

    if(actTime >= prevTick)
    {
        ElapsTime = actTime - prevTick;
    }
    else
    {
        ElapsTime = UINT32_MAX - prevTick + 1;
        ElapsTime += actTime;
    }

    return ElapsTime;
}

/**
  * @brief  按键事件绑定
  * @param  function: 回调函数指针
  * @retval 无
  */
void ButtonEvent::EventAttach(FuncCallback_t function)
{
    priv.eventCallback = function;
}

/**
  * @brief  监控事件，建议扫描周期10ms
  * @param  isPress: 是否按下
  * @retval 无
  */
void ButtonEvent::EventMonitor(bool isPress)
{
    if(priv.eventCallback == NULL)
    {
        return;
    }
    //表示按键刚刚被按下
    if (isPress && priv.nowState == STATE_NO_PRESS)
    {
        //当前状态
        priv.nowState = STATE_PRESS;

        IsPressed = true;//是否有过按键按下操作
        priv.lastPressTime = GET_TICK();
        //并触发EVENT_PRESSED和EVENT_CHANGED事件回调函数
        priv.eventCallback(this, EVENT_PRESSED);
        priv.eventCallback(this, EVENT_CHANGED);
    }
    //平常状态直接返回
    if(priv.nowState == STATE_NO_PRESS)
    {
        return;
    }
    //按键正在按下
    if(isPress)
    {
        //则触发EVENT_PRESSING事件回调函数
        priv.eventCallback(this, EVENT_PRESSING);
    }
    //长按按键
    if (isPress && GetTickElaps(priv.lastPressTime) >= priv.longPressTimeCfg)
    {
        //当前状态
        priv.nowState = STATE_LONG_PRESS;
        //第一次触发长按按键操作
        if(!priv.isLongPressed)
        {
            //则触发EVENT_LONG_PRESSED事件回调函数
            priv.eventCallback(this, EVENT_LONG_PRESSED);
            priv.lastLongPressTime = GET_TICK();
            IsLongPressed = true;//是否有过按键长按操作
            priv.isLongPressed = true;
        }
        //长按按键重复触发
        else if(GetTickElaps(priv.lastLongPressTime) >= priv.longPressRepeatTimeCfg)
        {
            priv.lastLongPressTime = GET_TICK();
            //则触发EVENT_LONG_PRESSED_REPEAT事件回调函数
            priv.eventCallback(this, EVENT_LONG_PRESSED_REPEAT);
        }
    }
    //按键松开
    else if (!isPress)
    {
        //当前状态
        priv.nowState = STATE_NO_PRESS;
        //双击操作
        if(GetTickElaps(priv.lastClickTime) < priv.doubleClickTimeCfg)
        {
            priv.clickCnt++;
            //则触发EVENT_DOUBLE_CLICKED事件回调函数
            priv.eventCallback(this, EVENT_DOUBLE_CLICKED);
        }
        //长按释放
        if(priv.isLongPressed)
        {
            //则触发EVENT_LONG_PRESSED_RELEASED事件回调函数
            priv.eventCallback(this, EVENT_LONG_PRESSED_RELEASED);
        }

        priv.isLongPressed = false;
        IsClicked = true;//是否有过点击操作
        priv.lastClickTime = GET_TICK();

        if(GetTickElaps(priv.lastPressTime) < priv.longPressTimeCfg)
        {
            //则触发EVENT_SHORT_CLICKED事件回调函数
            priv.eventCallback(this, EVENT_SHORT_CLICKED);
        }

        //触发EVENT_CLICKED、EVENT_RELEASED和EVENT_CHANGED事件回调函数
        priv.eventCallback(this, EVENT_CLICKED);
        priv.eventCallback(this, EVENT_RELEASED);
        priv.eventCallback(this, EVENT_CHANGED);
    }
}


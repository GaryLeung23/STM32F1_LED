/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "TrackLineFilter.h"
#include <string.h>
#include <math.h>

#define SQ(x) ((x) * (x))

#define TLF_USE_LOG 0

#if TLF_USE_LOG
#include <stdio.h>
#define LOG_PRINT(format, ...) printf(format, ##__VA_ARGS__), printf("\n")
#else
#define LOG_PRINT(format, ...)
#endif


TrackLineFilter::TrackLineFilter()
{
    memset(&priv, 0, sizeof(priv));
}

TrackLineFilter::~TrackLineFilter()
{
}
/**
 * @brief Reset Filter,并且发送EVENT_RESET事件
 */
void TrackLineFilter::Reset()
{
    priv.lineCount = 0;
    priv.pointCnt = 0;
    priv.pointOutputCnt = 0;
    priv.inArea = false;
    SendEvent(EVENT_RESET, nullptr);
}
/**
 * @brief 将datapoint 推入过滤器中，在clipArea内的点会被发送到callback处理
 * @param point
 */
void TrackLineFilter::PushPoint(const Point_t *point)
{
    //point在clipArea内
    if (GetIsPointInArea(&priv.clipArea, point))
    {
        //上一个point不在clipArea
        if (!priv.inArea)
        {
            const Point_t *p = priv.pointCnt > 0 ? &priv.prePoint : point;
            SendEvent(EVENT_START_LINE, p); //发送EVENT_START_LINE事件
            priv.inArea = true;
        }
        //output在clipArea内的点
        OutputPoint(point);
    }
    else
    {
        //上一个point在clipArea
        if (priv.inArea)
        {
            //发送EVENT_END_LINE事件
            SendEvent(EVENT_END_LINE, point);
            priv.lineCount++;// 增加Line 数量
            priv.inArea = false;
        }
    }
    priv.prePoint = *point;
    priv.pointCnt++;
}

/**
 * @brief 强制发送EVENT_APPEND_POINT事件，不增加pointOutputCnt
 * @param point
 */
void TrackLineFilter::PushPointForce(const Point_t *point)
{
    SendEvent(EVENT_APPEND_POINT, point);
}
/**
 * @brief 手动发送EVENT_END_LINE事件
 */
void TrackLineFilter::PushEnd()
{
    const Point_t *p = priv.pointCnt > 0 ? &priv.prePoint : nullptr;
    SendEvent(EVENT_END_LINE, p);
}

/**
 * @brief 设置clipArea
 * @param area
 */
void TrackLineFilter::SetClipArea(const Area_t *area)
{
    priv.clipArea = *area;
}

/**
 * @brief 设置OutputPointCallback
 * @param callback
 */
void TrackLineFilter::SetOutputPointCallback(Callback_t callback)
{
    priv.outputCallback = callback;
}
/**
 * @brief 判断point是否在area内
 * @param area
 * @param point
 * @return true表示在area内
 */
bool TrackLineFilter::GetIsPointInArea(const Area_t *area, const Point_t *point)
{
    bool inArea = (point->x >= area->x0 && point->x <= area->x1 && point->y >= area->y0 && point->y <= area->y1);

    LOG_PRINT(
        "# Point(%d,%d) Area(%d,%d # %d,%d) inArea:%d",
        point->x, point->y,
        area->x0, area->y0,
        area->x1, area->y1,
        inArea);

    return inArea;
}
/**
 * @brief 发送EVENT_APPEND_POINT事件，并增加pointOutputCnt
 * @param point
 */
void TrackLineFilter::OutputPoint(const Point_t *point)
{
    SendEvent(EVENT_APPEND_POINT, point);//
    priv.pointOutputCnt++;
}


/**
 * @brief 调用callback处理Event
 * @param code  eventcode
 * @param point datapoint
 */
void TrackLineFilter::SendEvent(EventCode_t code, const Point_t *point)
{
    if (!priv.outputCallback)
    {
        return;
    }

    Event_t event;
    event.code = code;
    event.lineIndex = priv.lineCount;
    event.point = point;
    priv.outputCallback(this, &event);
}

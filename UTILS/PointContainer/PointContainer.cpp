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
#include "PointContainer.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>

/*
 *	
 *
 */

PointContainer::PointContainer()
{
    memset(&priv, 0, sizeof(priv));
}

PointContainer::~PointContainer()
{

}

bool PointContainer::IsFlag(const DiffPoint_t* point)
{
    if (point->x == FLAG_END_POINT && point->y == FLAG_END_POINT)
    {
        return true;
    }

    if (point->x == FLAG_FULL_POINT && point->y == FLAG_FULL_POINT)
    {
        return true;
    }

    return false;
}
/**
 * @brief Push FullPoint to container
 * @param point FullPoint_t
 */
void PointContainer::PushPoint(const FullPoint_t* point)
{
    //container 为空时
    size_t curIndex = vecPoints.size();
    if (curIndex == 0)
    {
        priv.curPushPoint = *point;
        //Push
        PushFullPoint(point);
        return;
    }

    //计算diff值
    int32_t diffX = point->x - priv.curPushPoint.x;
    int32_t diffY = point->y - priv.curPushPoint.y;
    //记录当前点
    priv.curPushPoint = *point;

	//如果diff超过 signed char 的范围，那么就直接push FullPoint
    if (std::abs((int)diffX) > SCHAR_MAX || std::abs((int)diffY) > SCHAR_MAX)
    {
        //push
        PushFullPoint(point);
        return;
    }
    //制作diffPoint
    const DiffPoint_t diffPoint = { (int8_t)diffX, (int8_t)diffY };


    //这个判断感觉没有用
    //如果是FLAG Point，那么直接push FullPoint
    if (IsFlag(&diffPoint))
    {
        PushFullPoint(point);
        return;
    }

    vecPoints.push_back(diffPoint);
}

/**
 * @brief Push FullPoint to Container
 *        Container使用vector 容器
 *        注意这里的次序 FullPoint_t 转存到 DiffPoint_t的次序问题
 * @param point FullPoint_t
 */
void PointContainer::PushFullPoint(const FullPoint_t* point)
{
    //FLAG Point
    vecPoints.push_back(MakeFlag(FLAG_FULL_POINT));
    //FullPoint
    const DiffPoint_t* pData = (const DiffPoint_t*)point;
    for (int i = 0; i < sizeof(FullPoint_t) / sizeof(DiffPoint_t); i++)
    {
        vecPoints.push_back(pData[i]);
    }
}

/**
 * @brief Get FullPoint from container  according to curGetIndex
 *        Container使用vector 容器
 * @param point
 * @return true if success
 */
bool PointContainer::GetFullPoint(FullPoint_t* point)
{
    //检查size
    size_t size = vecPoints.size();
    if (size - priv.curGetIndex < 4)
    {
        return false;
    }

    DiffPoint_t* pData = (DiffPoint_t*)point;
    for (int i = 0; i < sizeof(FullPoint_t) / sizeof(DiffPoint_t); i++)
    {
        pData[i] = vecPoints[priv.curGetIndex];
        priv.curGetIndex++;
    }
    return true;
}

bool PointContainer::GetPoint(FullPoint_t* point)
{
    size_t size = vecPoints.size();
    //container 为空时
    if (size - priv.curGetIndex == 0)
    {
        return false;
    }

    //get diffPoint
    DiffPoint_t diffPoint = vecPoints[priv.curGetIndex];

    //如果是FLAG_FULL，那么直接push FullPoint
    if (diffPoint.x == FLAG_FULL_POINT && diffPoint.y == FLAG_FULL_POINT)
    {
        //跳过Flag
        priv.curGetIndex++;

        if (GetFullPoint(point))
        {
            //保存full point
            priv.curGetPoint = *point;
        }
        else
        {
            return false;
        }
    }
    else
    {
        //使用curGetPoint 计算出真实point
        priv.curGetPoint.x += diffPoint.x;
        priv.curGetPoint.y += diffPoint.y;
        *point = priv.curGetPoint;
        priv.curGetIndex++;
    }

    return true;
}

/**
 * @brief reset curGetIndex to 0
 */
void PointContainer::ResetGetIndex()
{
    priv.curGetIndex = 0;
}

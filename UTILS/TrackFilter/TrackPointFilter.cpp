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
#include "TrackPointFilter.h"
#include <string.h>
#include <cmath>

#define SQ(x) ((x) * (x))
#define FLOAT_0 0.00001

#define TPF_USE_LOG 0
//TPF_USE_LINE_CHECKED 会检查轨迹点是否在直线上，如果不在直线上则阻塞
#define TPF_USE_LINE_CHECKED 0   

#if TPF_USE_LOG
#include <stdio.h>
#define LOG_PRINT(format, ...) printf(format, ##__VA_ARGS__), printf("\n")
#else
#define LOG_PRINT(format, ...)
#endif

TrackPointFilter::TrackPointFilter()
{
    memset(&priv, 0, sizeof(priv));
}

TrackPointFilter::~TrackPointFilter()
{
}

void TrackPointFilter::Reset()
{
    priv.pointCnt = 0;
    priv.pointOutputCnt = 0;
}
/**
 * @brief 将轨迹点推入过滤器中，过于掉过于密集的点(根据priv.offsetThreshold)
 * @param point
 * @return point是有效点则返回true
 */
bool TrackPointFilter::PushPoint(const Point_t *point)
{
    bool retval = false;
    DumpPoint("\n+P", point);
    // 如果已输出轨迹点数量为0，则直接输出该点
    if (priv.pointCnt == 0)
    {
        retval = true;
        OutputPoint(point);
    }
    //如果已输出轨迹点数量为1，则计算出该点与前一个点之间的直线，并将该直线作为参考直线。
    else if (priv.pointCnt == 1)
    {
        if (!GetLine(&priv.refLine, &priv.prePoint, point))
        {
            return false;
        }
        DumpLine("First", &priv.refLine);
        retval = true;//补充一下，原代码没有这一句
    }
    //如果已输出轨迹点数量大于1
    else
    {
        DumpLine("--", &priv.refLine);

        //计算当前点与参考直线的距离
        double offset = GetOffset(&priv.refLine, point);
        LOG_PRINT("OFS = %lf", offset);

        //检查点是否大于有效范围
        if (offset > priv.offsetThreshold)
        {
            LOG_PRINT("<---> offset detect!");

            retval = true;

            //output tail point
            if (priv.secondFilterMode)
            {
                OutputPoint(&priv.tailPoint);
            }
            //output pre point
            OutputPoint(&priv.prePoint);
            //重新计算参考直线
            if (!GetLine(&priv.refLine, &priv.prePoint, point))
            {
                return false;
            }
        }
        //表示点不在有效范围里，去除(不output)前进方向上过于密集的点，回头点不去除
        else
        {
            Line_t line;
            //重新生成上一次的参考线段
            if (!GetLine(&line, &priv.tailPoint, &priv.prePoint))
            {
                return false;
            }

            DumpLine("L", &line);
#if TPF_USE_LINE_CHECKED
            bool inLine1 = GetIsPointInLine(&line, &priv.tailPoint);
            if (!inLine1)
            {
                DumpPoint("tailPoint", &priv.tailPoint);
                LOG_PRINT("not in L");
                while (1)
                    ;
            }

            bool inLine2 = GetIsPointInLine(&line, &priv.prePoint);
            if (!inLine2)
            {
                DumpPoint("prePoint", &priv.prePoint);
                LOG_PRINT("not in L");
                while (1)
                    ;
            }
#endif

            Line_t verLine;
            //在参考直线的终点(prePoint),作一条垂直于参考直线的直线
            GetVerticalLine(&verLine, &line, &priv.prePoint);

            DumpLine("|", &verLine);
            DumpPoint("in", &priv.prePoint);

#if TPF_USE_LINE_CHECKED
            bool inLine3 = GetIsPointInLine(&verLine, &priv.prePoint);
            if (!inLine3)
            {
                DumpPoint("prePoint", &priv.prePoint);
                LOG_PRINT("not in verLine");
                while (1)
                    ;
            }
#endif
            //判断当前点point与参考线段的起点(tailPoint)是否在垂直直线的同一侧，用在方向改变的判断
            if (GetIsOnSameSide(&verLine, &priv.tailPoint, point))
            {
                LOG_PRINT("~~~ direction change detect!");

                DumpPoint("p0", &priv.tailPoint);
                DumpPoint("p1", &priv.prePoint);
                DumpPoint("p2", point);

                retval = true;

                //output tail point
                if (priv.secondFilterMode)
                {
                    OutputPoint(&priv.tailPoint);
                }
                //output pre point
                OutputPoint(&priv.prePoint);
                //重新计算参考直线
                if (!GetLine(&priv.refLine, &priv.prePoint, point))
                {
                    return false;
                }
            }
        }
    }

    priv.tailPoint = priv.prePoint;//
    priv.prePoint = *point;//
    priv.pointCnt++;

    return retval;
}

/**
 * @brief 输出最后一个点,然后reset
 */
void TrackPointFilter::PushEnd()
{
    OutputPoint(&priv.prePoint);//输出最后一个点
    Reset();
}

/**
 * @brief 设置点到refLine距离的有效阈值
 * @param offset
 */
void TrackPointFilter::SetOffsetThreshold(double offset)
{
    priv.offsetThreshold = offset;
}

/**
 * @brief 设置callback
 * @param callback
 */
void TrackPointFilter::SetOutputPointCallback(Callback_t callback)
{
    priv.outputCallback = callback;
}

/**
 * @brief 设置二次过滤模式 默认关闭
 * @param en
 */
void TrackPointFilter::SetSecondFilterModeEnable(bool en)
{
    priv.secondFilterMode = en;
}

//
/**
 * @brief 调用callback处理point，并且增加pointOutputCnt
 * @param point
 */
void TrackPointFilter::OutputPoint(const Point_t *point)
{
    if (priv.outputCallback)
    {
        DumpPoint(">>> output", point);
        LOG_PRINT("");
        priv.outputCallback(this, point);
    }
    priv.pointOutputCnt++;
}

/**
 * @brief 两点确定一条直线
 * @param line  [out]
 * @param point0
 * @param point1
 * @return 成功返回true，如果是同一个点就返回false
 */
bool TrackPointFilter::GetLine(Line_t *line, const Point_t *point0, const Point_t *point1)
{
    bool retval = true;

    double x0 = point0->x;
    double x1 = point1->x;
    double y0 = point0->y;
    double y1 = point1->y;

    double x_diff_abs = std::abs(x0 - x1);
    double y_diff_abs = std::abs(y0 - y1);

    double a = 0;
    double b = 0;
    double c = 0;

    // x= x0 => x+c =0,c=-x0
    if (x_diff_abs < FLOAT_0 && y_diff_abs > FLOAT_0)
    {
        a = 1;
        b = 0;
        c = -x0;
    }
    // y = y0 => y+c =0,c=-y0
    else if (x_diff_abs > FLOAT_0 && y_diff_abs < FLOAT_0)
    {
        a = 0;
        b = 1;
        c = -y0;
    }
    //一般形式 (y-y0)/(y1-y0) = (x-x0)/(x1-x0)
    else if (x_diff_abs > FLOAT_0 && y_diff_abs > FLOAT_0)
    {
        a = (y1 - y0) / (x0 - x1);
        b = (a * (x0 - x1)) / (y1 - y0); //1
        c = 0 - a * x0 - b * y0;
    }
    // 同一个点
    else
    {
        retval = false;
    }

    line->a = a;
    line->b = b;
    line->c = c;

    return retval;
}

void TrackPointFilter::DumpLine(const char *name, const Line_t *line)
{
    LOG_PRINT(
        "%s : %lfx + %lfy + %lf = 0 { y = %lfx + %lf }",
        name, line->a, line->b, line->c,
        -line->a / line->b, -line->c / line->b);
}

void TrackPointFilter::DumpPoint(const char *name, const Point_t *point)
{
    LOG_PRINT("%s : (%lf, %lf)", name, point->x, point->y);
}

/**
 * @brief 通过oriLine获取垂直于oriLine的直线verLine
 *        两直线垂直，斜率乘积为-1;oriline的斜率是-a/b,verline的斜率是b/a
 * @param verLine [out]
 * @param oriLine
 * @param point 垂直点
 */
void TrackPointFilter::GetVerticalLine(Line_t *verLine, const Line_t *oriLine, const Point_t *point)
{
    verLine->a = -oriLine->b;
    verLine->b = oriLine->a;
    verLine->c = 0 - verLine->a * point->x - verLine->b * point->y;
}

/**
 * @brief 计算点到直线的距离
 *        点到直线的距离公式：d = |ax0 + by0 + c| / √(a² + b²)
 * @param line
 * @param point
 * @return 点到直线的距离
 */
double TrackPointFilter::GetOffset(const Line_t *line, const Point_t *point)
{
    double temp = line->a * point->x + line->b * point->y + line->c;
    double offset = std::abs(temp) * QuickRSqrt(SQ(line->a) + SQ(line->b));
    return offset;
}

/**
 * @brief 判断两点是否在直线的同一侧，符号是否异号
 * @param line
 * @param point0
 * @param point1
 * @return  true表示在同一侧，false表示不在同一侧
 */
bool TrackPointFilter::GetIsOnSameSide(const Line_t *line, const Point_t *point0, const Point_t *point1)
{
    bool retval = true;
    double side = (line->a * point0->x + line->b * point0->y + line->c) * (line->a * point1->x + line->b * point1->y + line->c);

    //符号是否异号
    if (side < FLOAT_0)
    {
        retval = false;
    }

    return retval;
}

/**
 * @brief 判断点是否在直线上
 * @param line
 * @param point
 * @return true表示在直线上，false表示不在直线上
 */
bool TrackPointFilter::GetIsPointInLine(const Line_t *line, const Point_t *point)
{
    double result = line->a * point->x + line->b * point->y + line->c;
    return std::abs(result) < FLOAT_0;
}


/**
 * @brief 使用"Fast Inverse Square Root"算法来快速计算输入数值的平方根的倒数
 * @param num
 * @return num的平方根的倒数
 */
double TrackPointFilter::QuickRSqrt(double num)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5f;
    x2 = (float)num * 0.5f;
    y = (float)num;
    i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y));
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

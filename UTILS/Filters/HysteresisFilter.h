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
#ifndef __HYSTERESIS_FILTER_H
#define __HYSTERESIS_FILTER_H

#include "FilterBase.h"

//滞回滤波器可以有效地去除瞬间的干扰信号，具有较好的平滑效果。
//定义了一个hysValue变量表示滞回值，当当前数据与上一次数据之差超过滞回值时，当前数据会覆盖上一次数据，否则保留上一次数据。

namespace Filter {

    template<typename T>
    class Hysteresis : public Base<T> {
    public:
        Hysteresis(T hysVal) {
            this->Reset();
            this->hysValue = hysVal;
        }

        virtual T GetNext(T value) {
            if (this->isFirst) {
                this->isFirst = false;
                this->lastValue = value;
            } else {
                if (FILTER_ABS(value - this->lastValue) > this->hysValue) {
                    this->lastValue = value;
                }
            }

            return this->lastValue;
        }

    private:
        T hysValue;
    };

}

#endif

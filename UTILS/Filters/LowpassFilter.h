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
#ifndef __LOWPASS_FILTER_H
#define __LOWPASS_FILTER_H

#include "FilterBase.h"

//低通滤波器类Lowpass，该滤波器可以有效地滤去高频噪声，使得信号变得平滑。
//具体来讲，当一个新数据输入时，函数会根据上一次的输出进行计算，计算结果会依据RC值进行加权平均，得到最终的输出值。

namespace Filter {

    template<typename T>
    class Lowpass : public Base<T> {
    public:
        //dt代表采样周期，cutoff代表截止频率。
        Lowpass(float dt, float cutoff) {
            this->Reset();

            this->dT = dt;
            if (cutoff > 0.001f) {
                float RC = 1 / (2 * 3.141592653f * cutoff);
                this->rc = dt / (RC + dt);
            } else {
                this->rc = 1;
            }
        }

        virtual T GetNext(T value) {
            if (this->CheckFirst()) {
                return this->lastValue = value;
            } else {
                this->lastValue = (this->lastValue + (value - this->lastValue) * this->rc);
                return this->lastValue;
            }
        }

    private:
        float dT;
        float rc;
    };

}

#endif

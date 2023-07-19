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
#ifndef __MEDIAN_QUEUE_FILTER_H
#define __MEDIAN_QUEUE_FILTER_H

#include "MedianFilter.h"

//中值滤波器的一个变种——中值队列滤波器。
//不需要等bufferSize大小的数据，每输入一个数据就会进行一次滤波

namespace Filter {

    template<typename T, size_t bufferSize>
    class MedianQueue : public Median<T, bufferSize> {
    public:
        MedianQueue() : Median<T, bufferSize>() {
            this->realIndex = 0;
        }

        virtual T GetNext(T value) {
            size_t size;
            if (bufferSize == 0) {
                return 0;
            }
            this->dataIndex = this->realIndex % bufferSize;
            this->bufferSort[this->dataIndex] = value;
            this->realIndex++;
            if (this->realIndex < bufferSize) {
                size = this->realIndex;
            } else {
                size = bufferSize;
            }
            std::sort(this->bufferSort, this->bufferSort + size);
            this->lastValue = this->bufferSort[size / 2];


            return this->lastValue;
        }

    protected:
        T bufferSort[bufferSize];
        size_t realIndex;
    };

}

#endif

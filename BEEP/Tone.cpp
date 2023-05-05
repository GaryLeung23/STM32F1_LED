/*
 * MIT License
 * Copyright (c) 2017 - 2022 _VIFEXTech
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
#include "Tone.h"

#define TONE_FREQ_MAX 500000U //1Mhz /2


static bool Tone_IsContinuousModeEnable = false;
static uint16_t Tone_Pin = 0;
static GPIO_TypeDef* Tone_GPIO = NULL;
static uint32_t Tone_ToggleCounter = 0;

/**
  * @brief  tone中断入口，被定时中断调用
  * @param  无
  * @retval 无
  */
void Tone_TimerHandler(void)
{
    if(!Tone_IsContinuousModeEnable)
    {
        if(Tone_ToggleCounter == 0)
        {
            noTone(Tone_GPIO,Tone_Pin);
            return;
        }

        Tone_ToggleCounter--;
    }

    HAL_GPIO_TogglePin(Tone_GPIO,Tone_Pin);
}


/**
  * @brief  在Pin上生成指定频率 (50%占空比的方波)
  * @param  pin: 产生音调的 Pin
  * @param  freq: 频率(Hz)
  * @param  duration: 音调的持续时间 (以毫秒为单位)
  * @retval 无
  */
void tone(GPIO_TypeDef* gpio ,uint16_t pin, uint32_t freq, uint32_t duration)
{
    noTone(gpio,pin);

    if(duration == 0 || freq == 0 || freq > TONE_FREQ_MAX)
    {
        return;
    }

    Tone_Pin = pin;
    Tone_GPIO = gpio;
    Tone_IsContinuousModeEnable = (duration == TONE_DURATION_INFINITE) ? true : false;

    if(!Tone_IsContinuousModeEnable)
    {
        Tone_ToggleCounter = freq * duration / 1000 * 2;// /1000是因为duration的单位是ms *2是因为占空比为50% Toggle次数

        if(Tone_ToggleCounter == 0)
        {
            return;
        }

        Tone_ToggleCounter--;
    }

    __HAL_TIM_SET_AUTORELOAD(&TONE_TIMER, TONE_FREQ_MAX / freq); //unit:us
    __HAL_TIM_SET_COUNTER(&TONE_TIMER, 0);

    HAL_TIM_Base_Start_IT(&TONE_TIMER);
}

/**
  * @brief  关闭声音
  * @param  Pin: 产生音调的引脚编号
  * @retval 无
  */
void noTone(GPIO_TypeDef* gpio ,uint16_t pin)
{
    HAL_TIM_Base_Stop_IT(&TONE_TIMER);

    HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);//低电平无效
    Tone_IsContinuousModeEnable = false;
    Tone_Pin = 0;
    Tone_GPIO = NULL;
    Tone_ToggleCounter = 0;
}

#include "main.h"
#include "Tone.h"
#include "Buzz.h"

static bool IsEnable = true;

void Buzz_init()
{
    //GPIO INIT
}
void Buzz_SetEnable(bool en)
{
    if(!en)
    {
        Buzz_Tone(0);
    }

    IsEnable = en;
}

void Buzz_Tone(uint32_t freq, int32_t duration)
{
    if(!IsEnable)
    {
        return;
    }

    if(duration >= 0)
    {
        tone(BEEP_GPIO_Port,BEEP_Pin, freq, duration);
    }
    else
    {
        tone(BEEP_GPIO_Port,BEEP_Pin, freq,TONE_DURATION_INFINITE);
    }
}


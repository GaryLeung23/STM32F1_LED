#ifndef __BUZZ_H
#define __BUZZ_H


#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Buzzer */
void Buzz_init();
void Buzz_SetEnable(bool en);
void Buzz_Tone(uint32_t freq, int32_t duration = -1);



#ifdef __cplusplus
}
#endif


#endif /* __BUZZ_H */
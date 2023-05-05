#ifndef __AUDIO_H
#define __AUDIO_H
#include "stdio.h"
#include "stm32f1xx_hal.h"

#define millis()    HAL_GetTick()

#ifdef __cplusplus
extern "C" {
#endif

/* Audio */
void Audio_Init();
void Audio_Update();
bool Audio_PlayMusic(const char* name);

#ifdef __cplusplus
}
#endif


#endif /* __AUDIO_H */
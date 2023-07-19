#ifndef __SDCARD_H
#define __SDCARD_H

#include "main.h"
#include "HAL_Def.h"
#include "stdio.h"
#include "sdio.h"
#include "../UTILS/Macro/CommonMacro.h"
#include "Audio.h"
#include "Config.h"
#include "fatfs.h"
#include "string.h"


extern const char *SD_GetTypeName();

extern bool SD_Init();

extern bool SD_GetReady();

extern void SD_SetEventCallback(HAL::SD_CallbackFunction_t callback);

extern void SD_Update();

extern float SD_GetCardSizeMB();


#endif /* __SDCARD_H */
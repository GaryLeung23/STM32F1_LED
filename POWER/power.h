#ifndef __POWER_H
#define __POWER_H




#include "adc.h"
#include "main.h"
#include "HAL_Def.h"
#include "../UTILS/Macro/CommonMacro.h"
#include "string.h"
#include "stdio.h"
#include "Config.h"


extern void Power_Init();

extern void Power_Shutdown();

extern void Power_Update();

extern void Power_EventMonitor();

extern void Power_GetInfo(HAL::Power_Info_t *info);

extern void Power_SetEventCallback(HAL::Power_CallbackFunction_t callback);




#endif /* __POWER_H */
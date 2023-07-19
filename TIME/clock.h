#ifndef __CLOCK_H
#define __CLOCK_H

#include "HAL_Def.h"
#include "rtc.h"

extern void Clock_SetInfo(const HAL::Clock_Info_t *info);

extern void Clock_GetInfo(HAL::Clock_Info_t *info);


#endif /* __CLOCK_H */
#ifndef __GPS_H
#define __GPS_H




#include "HAL_Def.h"
#include "string.h"


extern void GPS_Init();

extern void GPS_Update();

extern bool GPS_GetInfo(HAL::GPS_Info_t *info);

extern bool GPS_LocationIsValid();

extern double GPS_GetDistanceOffset(HAL::GPS_Info_t *info, double preLong, double preLat);



#endif // __GPS_H
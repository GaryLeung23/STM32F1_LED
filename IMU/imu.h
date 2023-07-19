#ifndef __IMU_H
#define __IMU_H



#include "HAL_Def.h"

extern bool IMU_Init();

extern void IMU_Update();

extern void IMU_SetCommitCallback(HAL::CommitFunc_t func, void *userData);



#endif  // __IMU_H
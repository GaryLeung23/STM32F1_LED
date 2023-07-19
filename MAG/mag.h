#ifndef __MAG_H
#define __MAG_H




#include "HAL_Def.h"


extern bool MAG_Init();

extern void MAG_Update();

extern void MAG_SetCommitCallback(HAL::CommitFunc_t func, void *userData);




#endif  // __MAG_H
#include "mag.h"

static HAL::CommitFunc_t CommitFunc = nullptr;
static void *UserData = nullptr;

bool MAG_Init() {
    //TODO
    return false;
}

void MAG_SetCommitCallback(HAL::CommitFunc_t func, void *userData) {
    CommitFunc = func;
    UserData = userData;
}

void MAG_Update() {
    //TODO
    HAL::MAG_Info_t magInfo = {0};


    if (CommitFunc) {
        CommitFunc(&magInfo, UserData);
    }
}

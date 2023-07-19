#include "imu.h"


static HAL::CommitFunc_t CommitFunc = nullptr;
static void *UserData = nullptr;

bool IMU_Init() {
    //TODO
    return false;
}

void IMU_SetCommitCallback(HAL::CommitFunc_t func, void *userData) {
    CommitFunc = func;
    UserData = userData;
}

void IMU_Update() {
    HAL::IMU_Info_t imuInfo = {0};
    //TODO

    if (CommitFunc) {
        CommitFunc(&imuInfo, UserData);
    }
}

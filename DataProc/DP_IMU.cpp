#include "DataProc.h"
#include "imu.h"

DATA_PROC_INIT_DEF(IMU) {
    IMU_SetCommitCallback([](void *info, void *userData) {
        Account *account = (Account *) userData;
        return account->Commit(info, sizeof(HAL::IMU_Info_t));//commit imu info to cache
    }, account);
    //没有设置SetEventCallback，当其他account pull数据的时候会直接从cache中返回。
}

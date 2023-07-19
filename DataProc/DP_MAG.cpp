#include "DataProc.h"
#include "mag.h"

DATA_PROC_INIT_DEF(MAG) {
    MAG_SetCommitCallback([](void *info, void *userData) {
        Account *account = (Account *) userData;
        return account->Commit(info, sizeof(HAL::MAG_Info_t));
    }, account);
    //没有设置SetEventCallback，当其他account pull数据的时候会直接从cache中返回。
}

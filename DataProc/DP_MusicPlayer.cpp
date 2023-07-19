#include "DataProc.h"
#include "Audio.h"

static int onEvent(Account *account, Account::EventParam_t *param) {
    if (param->event != Account::EVENT_NOTIFY) {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (param->size != sizeof(DataProc::MusicPlayer_Info_t)) {
        return Account::RES_SIZE_MISMATCH;
    }

    DataProc::MusicPlayer_Info_t *info = (DataProc::MusicPlayer_Info_t *) param->data_p;

    Audio_PlayMusic(info->music);//播放音乐

    return Account::RES_OK;
}

DATA_PROC_INIT_DEF(MusicPlayer) {
    account->SetEventCallback(onEvent);
}

#include "sdcard.h"

static HAL_SD_CardInfoTypeDef CardInfo = {0};
static bool SD_IsReady = false;
static uint32_t SD_CardSize = 0;
static HAL::SD_CallbackFunction_t SD_EventCallback = nullptr;

static bool SD_CheckDir(const char *path) {

    DIR dir;
    FRESULT f_res;
    char abspath[32] = "0:/";
    strcat(abspath, path);
    f_res = f_opendir(&dir, abspath);
    if (f_res != FR_OK) {
        return false;
    }
    f_closedir(&dir);
    return true;
}

float SD_GetCardSizeMB() {
    return SD_CardSize / 1024 / 1024;
}

const char *SD_GetTypeName() {
    const char *type = "Unknown";

    if (!SD_CardSize) {
        goto failed;
    }

    switch (CardInfo.CardType) {
        case CARD_SDSC:
            type = "SDSC";
            break;

        case CARD_SDHC_SDXC:
            type = ((SD_GetCardSizeMB() / 1024.0f) < 32) ? "SDHC" : "SDXC";
            break;

        default:
            break;
    }

    failed:
    return type;
}


bool SD_Init() {
    bool retval = true;
    bool res;

    //TODO
//    init sdcard
//    printf("SD: init...\r\n");

// get sdcard info


    if (HAL_SD_GetCardInfo(&hsd, &CardInfo) != HAL_OK) {
        printf("SD: CARD was not inserted\r\n");
        retval = false;
    }
    if (retval) {
        SD_CardSize = CardInfo.BlockNbr * CardInfo.BlockSize;

        res = SD_CheckDir(CONFIG_TRACK_RECORD_FILE_DIR_NAME);
        if(!res)
            printf("SD_CheckDir failed\r\n");
        else
            printf("SD_CheckDir success\r\n");

        printf(
                "success, Type: %s, Size: %0.2f GB\r\n",
                SD_GetTypeName(),
                SD_GetCardSizeMB() / 1024.0f
        );
    } else {
        printf("failed: 0x%x\r\n", retval);
    }
    SD_IsReady = retval;

    return retval;
}

bool SD_GetReady() {
    return SD_IsReady;
}

static void SD_Check(bool isInsert) {
    if (isInsert) {
        bool ret = SD_Init();

        if (ret && SD_EventCallback) {
            SD_EventCallback(true);
        }

        Audio_PlayMusic(ret ? "DeviceInsert" : "Error");
    } else {
        SD_IsReady = false;

        if (SD_EventCallback) {
            SD_EventCallback(false);
            SD_CardSize = 0;
        }

        Audio_PlayMusic("DevicePullout");
    }
}

void SD_SetEventCallback(HAL::SD_CallbackFunction_t callback) {
    SD_EventCallback = callback;
}

void SD_Update() {
    //TODO
//    bool isInsert = (digitalRead(CONFIG_SD_CD_PIN) == LOW);
//
//    CM_VALUE_MONITOR(isInsert, SD_Check(isInsert));
}

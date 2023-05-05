
#include "../../LVGL/lvgl.h"
#include "stdio.h"
#include "../lvgl_user.h"

uint8_t lvgl_fs_readfile(void) {
    lv_fs_file_t f;
    lv_fs_res_t res;
    res = lv_fs_open(&f, "/my_folder/FatFs_STM32.txt", LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK){
        printf("lv_fs_open error\n");
        printf("res = %d\n", res);
        goto openerr;
    }

    uint32_t read_num;
    uint8_t buf[256];
    res = lv_fs_read(&f, buf, 256, &read_num);
    if (res != LV_FS_RES_OK || read_num == 0) {
        printf("lv_fs_read error\n");
        printf("res = %d\n", res);
        goto readerr;
    }

    printf("read_num = %d\n", read_num);
    printf("buf = %s\n", buf);
readerr:
    lv_fs_close(&f);
openerr:
    return res;
}

uint8_t lvgl_fs_readdir(void){
    lv_fs_dir_t dir;
    lv_fs_res_t res;
    res = lv_fs_dir_open(&dir, "/my_folder");
    if(res != LV_FS_RES_OK){
        printf("lv_fs_dir_open error\n");
        printf("res = %d\n", res);
        goto  openerr;
    }
    printf("lv_fs_dir_open success\n");
    char fn[256];
    while(1) {
        res = lv_fs_dir_read(&dir, fn);
        if(res != LV_FS_RES_OK) {
            printf("lv_fs_dir_read error\n");
            printf("res = %d\n", res);
            goto readerr;
            break;
        }
/*fn is empty, if not more files to read*/
        if(strlen(fn) == 0) {
            break;
        }
        printf("%s\n", fn);
    }
readerr:
    lv_fs_dir_close(&dir);
openerr:
    return res;
}
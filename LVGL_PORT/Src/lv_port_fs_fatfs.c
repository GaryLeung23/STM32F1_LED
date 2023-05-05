/**
 * @file lv_port_fs_templ.c
 *
 */

/*Copy this file as "lv_port_fs.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_fs_fatfs.h"
#include "../../lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);

static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p);

static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);

static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw);

static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence);

static lv_fs_res_t fs_size(lv_fs_drv_t *drv, void *file_p, uint32_t *size_p);

static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);

static void *fs_dir_open(lv_fs_drv_t *drv, const char *path);

static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *rddir_p, char *fn);

static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *rddir_p);

/**********************
 *  STATIC VARIABLES
 **********************/
FATFS lv_fs;//这需要全局变量或者static变量
const TCHAR lv_fs_drv_path[4] = "0:/";
/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_fs_init(void) {
    /*----------------------------------------------------
     * Initialize your storage device and File System
     * -------------------------------------------------*/
    fs_init();

    /*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

    /*Add a simple drive to open images*/
    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);



    /*Set up fields...*/
    fs_drv.letter = FS_FATFS_LETTER;
    fs_drv.cache_size = FS_FATFS_CACHE_SIZE;

    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    fs_drv.dir_close_cb = fs_dir_close;
    fs_drv.dir_open_cb = fs_dir_open;
    fs_drv.dir_read_cb = fs_dir_read;

    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your Storage device and File system.*/
static void fs_init(void) {
    /*E.g. for FatFS initialize the SD card and FatFS itself*/
    FRESULT res;

    res = f_mount(&lv_fs, lv_fs_drv_path, 1);
    printf("%s res = %d\n", __func__, res);
    if (res == FR_NO_FILESYSTEM) {
        res = f_mkfs("0:", 0, 0);
        if (res == FR_OK) {

            printf("The SD card successfully formatted the file system\r\n");
            res = f_mount(NULL, lv_fs_drv_path, 1);

            res = f_mount(&lv_fs, lv_fs_drv_path, 1);
        } else {
            printf("The SD card failed to format the file system\r\n");
        }
    } else if (res != FR_OK) {
        printf(" mount error : %d \r\n", res);
        return;
    } else {
        printf(" mount ok \r\n");
    }
    /*You code here*/
}


/**
 * Open a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode      read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return          a file descriptor or NULL on error
 */
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
    LV_UNUSED(drv);
    uint8_t flags = 0;


    if (mode == LV_FS_MODE_WR) {
        /*Open a file for write*/
        flags = FA_WRITE | FA_OPEN_ALWAYS;
    } else if (mode == LV_FS_MODE_RD) {
        /*Open a file for read*/
        flags = FA_READ;
    } else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) {
        /*Open a file for read and write*/
        flags = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;
    }

    FIL *f = lv_malloc(sizeof(FIL));
    if (f == NULL)
        return NULL;

    FRESULT res = f_open(f, path, flags);
    if (res == FR_OK) {
        return f;
    } else {
        lv_free(f);
        return NULL;
    }
}

/**
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p) {
    LV_UNUSED(drv);
    f_close(file_p);
    lv_free(file_p);
    return LV_FS_RES_OK;
}

/**
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br) {
    LV_UNUSED(drv);
    FRESULT res = f_read(file_p, buf, btr, (UINT *) br);
    if (res == FR_OK)
        return LV_FS_RES_OK;
    else
        return LV_FS_RES_UNKNOWN;
}

/**
 * Write into a file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btw       Bytes To Write
 * @param bw        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw) {
    LV_UNUSED(drv);
    FRESULT res = f_write(file_p, buf, btw, (UINT *) bw);
    if (res == FR_OK)
        return LV_FS_RES_OK;
    else
        return LV_FS_RES_UNKNOWN;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence) {
    LV_UNUSED(drv);
    switch (whence) {
        case LV_FS_SEEK_SET:
            f_lseek(file_p, pos);
            break;
        case LV_FS_SEEK_CUR:
            f_lseek(file_p, f_tell((FIL *) file_p) + pos);
            break;
        case LV_FS_SEEK_END:
            f_lseek(file_p, f_size((FIL *) file_p) + pos);
            break;
        default:
            break;
    }
    return LV_FS_RES_OK;

}

/**
 * Give the position of the read write pointer
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param pos_p     pointer to to store the result
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
    LV_UNUSED(drv);
    *pos_p = f_tell((FIL *) file_p);
    return LV_FS_RES_OK;
}

/**
 * Initialize a 'lv_fs_dir_t' variable for directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to a directory
 * @return          pointer to the directory read descriptor or NULL on error
 */
static void *fs_dir_open(lv_fs_drv_t *drv, const char *path) {
    LV_UNUSED(drv);
    DIR * d = lv_malloc(sizeof(DIR));
    if(d == NULL)
        return NULL;

    FRESULT res = f_opendir(d, path);
    if(res != FR_OK) {
        printf("f_opendir failed res:%d\n",res);
        lv_free(d);
        d = NULL;
    }
    return d;
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @param fn        pointer to a buffer to store the filename
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *rddir_p, char *fn) {
    LV_UNUSED(drv);
    FRESULT res;
    FILINFO fno = {0};//局部变量注意初始化
    fn[0] = '\0';

    do {
        res = f_readdir(rddir_p, &fno);
        if(res != FR_OK)
            return LV_FS_RES_UNKNOWN;

        if(fno.fattrib & AM_DIR) {
            fn[0] = '/';
            strcpy(&fn[1], fno.fname);
        }
        else
            strcpy(fn, fno.fname);

    } while(strcmp(fn, "/.") == 0 || strcmp(fn, "/..") == 0);

    return LV_FS_RES_OK;
}

/**
 * Close the directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *rddir_p) {
    LV_UNUSED(drv);
    f_closedir(rddir_p);
    lv_free(rddir_p);
    return LV_FS_RES_OK;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif

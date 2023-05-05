/**
 * @file lv_port_fs_templ.h
 *
 */

/*Copy this file as "lv_port_fs.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_FS_TEMPL_H
#define LV_PORT_FS_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lvgl/lvgl.h"
#include "fatfs.h"
/*********************
 *      DEFINES
 *********************/
#define FS_FATFS_LETTER         '/'   //定义自己的volume name，不需要fatfs中的真实盘号对应。
#define FS_FATFS_CACHE_SIZE     0
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_fs_init(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_FS_TEMPL_H*/

#endif /*Disable/Enable content*/

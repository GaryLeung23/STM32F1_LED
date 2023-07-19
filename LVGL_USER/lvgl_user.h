/**
 * @file lv_examples.h
 *
 */

#ifndef LV_EXAMPLES_H
#define LV_EXAMPLES_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../LVGL/lvgl.h"
#include "../../LVGL_PORT/Inc/lv_port_indev.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern uint8_t lvgl_fs_readfile(void);

extern uint8_t lvgl_fs_readdir(void);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_EXAMPLES_H*/

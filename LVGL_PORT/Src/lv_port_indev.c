/**
 * @file lv_port_indev_templ.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "../Inc/lv_port_indev.h"
#include "../../LVGL/lvgl.h"
#include "../../LCD/Inc/XPT2046.h"
#include "../../KEY/Inc/key.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void touchpad_init(void);

static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

static bool touchpad_is_pressed(void);

static void touchpad_get_xy(lv_coord_t *x, lv_coord_t *y);

static void keypad_init(void);

static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

static uint32_t keypad_get_key(void);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t *indev_touchpad;
lv_indev_t *indev_keypad;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void) {
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    static lv_indev_drv_t indev_drv_touchpad;
    static lv_indev_drv_t indev_drv_keypad;

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv_touchpad);
    indev_drv_touchpad.type = LV_INDEV_TYPE_POINTER;
    indev_drv_touchpad.read_cb = xpt2046_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv_touchpad);


    /*------------------
     * Keypad
     * -----------------*/

    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    lv_indev_drv_init(&indev_drv_keypad);
    indev_drv_keypad.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_keypad.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv_keypad);

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/

    lv_group_t *group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);
    lv_group_set_default(group);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static void touchpad_init(void) {
    /*Your code comes here*/
    //XPT2046_Init();
}

/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    /*Save the pressed coordinates and the state*/
    if (touchpad_is_pressed()) {
        touchpad_get_xy(&last_x, &last_y);
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}

/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void) {
    /*Your code comes here*/


    return false;
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(lv_coord_t *x, lv_coord_t *y) {
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}


/*------------------
 * Keypad
 * -----------------*/

/*Initialize your keypad*/
static void keypad_init(void) {
    /*Your code comes here*/
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    static uint32_t last_key = 0;

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();

    if (act_key != 0) {
//        printf("act_key: %d\n", act_key);
        data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch (act_key) {
            case MY_KEY_DOWN:
                if(GetKeyUserMode() !=  MODE_GROUP)
                    act_key = LV_KEY_DOWN;
                else
                    act_key = LV_KEY_PREV;
//                act_key = LV_KEY_PREV;
                break;
            case MY_KEY_UP:
                if(GetKeyUserMode() !=  MODE_GROUP)
                    act_key = LV_KEY_UP;
                else
                    act_key = LV_KEY_NEXT;
//                act_key = LV_KEY_NEXT;
                break;
            case MY_KEY_RIGHT:
                act_key = LV_KEY_ENTER;
                break;
        }

        last_key = act_key;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void) {
    /*Your code comes here*/

    return Key_Scan();
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif

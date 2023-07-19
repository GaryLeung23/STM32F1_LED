#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MODE_GROUP = 0, //prev next
    MODE_SLIDER,    //UP DOWN
} key_user_mode_t;

static  key_user_mode_t  key_user_mode = MODE_GROUP;

#define MY_KEY0        HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin)
#define MY_KEY1        HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)
#define MY_KEY2        HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)
#define MY_KEY3        HAL_GPIO_ReadPin(KEY3_WAKEUP_GPIO_Port,KEY3_WAKEUP_Pin)


#define MY_KEY_UP       4
#define MY_KEY_LEFT     3
#define MY_KEY_DOWN     2
#define MY_KEY_RIGHT    1
#define MY_KEY_NONE     0



extern uint8_t Key_Scan(void);    //°´¼üÉ¨Ãèº¯Êý
extern void SetKeyUserMode(key_user_mode_t mode);
extern key_user_mode_t GetKeyUserMode(void);

#ifdef __cplusplus
}
#endif

#endif

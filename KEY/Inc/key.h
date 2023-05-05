#ifndef __KEY_H
#define __KEY_H	 
#include "main.h"
#include <stdio.h>
#include <stdbool.h>

#define key_delay_ms(m)             HAL_Delay(m)

#define MY_KEY0         HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin)
#define MY_KEY1 		HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)
#define MY_KEY2 		HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)
#define MY_KEY3 		HAL_GPIO_ReadPin(KEY3_WAKEUP_GPIO_Port,KEY3_WAKEUP_Pin)


#define MY_KEY_UP       4
#define MY_KEY_LEFT     3
#define MY_KEY_DOWN     2
#define MY_KEY_RIGHT    1
#define MY_KEY_NONE     0


#define SET_KEY_UNBOUNCE_FLAG(x) (key_unbounce_flag = x)
#define GET_KEY_UNBOUNCE_FLAG() (key_unbounce_flag)

extern bool key_unbounce_flag;
extern uint8_t Key_Scan_unbounce(bool continuous_mode);  	//°´¼üÉ¨Ãèº¯Êý
extern uint8_t Key_Scan(void);  	//°´¼üÉ¨Ãèº¯Êý
extern void Key_Update(void);
#endif

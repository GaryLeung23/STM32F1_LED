#include "../Inc/key.h"



//按键处理函数
//返回按键值
//continuous_mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，KEY3按下 WK_UP



//uint8_t Key_Scan_unbounce(bool continuous_mode) {
//    static uint8_t key_release = 1;//按键按松开标志
//    if (continuous_mode)
//        key_release = 1;  //支持连按
//    if (key_release && (MY_KEY0 == 0 || MY_KEY1 == 0 || MY_KEY2 == 0 || MY_KEY3 == 1)) {
//        key_delay_ms(5);//去抖动
//        key_release = 0;
//        if (MY_KEY0 == 0)
//            return MY_KEY_RIGHT;
//        else if (MY_KEY1 == 0)
//            return MY_KEY_DOWN;
//        else if (MY_KEY2 == 0)
//            return MY_KEY_LEFT;
//        else if (MY_KEY3 == 1)
//            return MY_KEY_UP;
//    } else if (MY_KEY0 == 1 && MY_KEY1 == 1 && MY_KEY2 == 1 && MY_KEY3 == 0)
//        key_release = 1;
//    return 0;// 无按键按下
//}


bool key_unbounce_flag = true;


//三行代码消抖    ，当按下按键的时候，只有一次Trg=1.
uint8_t Key1_down(void) {
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;

    uint8_t readdata = MY_KEY1 ^ 1;                   //按下是低电平,就需要取反
    Trg = readdata & (readdata ^ Cont);              //核心代码2
    Cont = readdata;                                //核心代码3

    return Trg;
}
uint8_t Key2_down(void){
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;

    uint8_t readdata = MY_KEY2 ^ 1;                   //按下是低电平,就需要取反
    Trg = readdata & (readdata ^ Cont);              //核心代码2
    Cont = readdata;                                //核心代码3

    return Trg;
}
uint8_t Key0_down(void){
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;

    uint8_t readdata = MY_KEY0 ^ 1;                  //按下是低电平,就需要取反
    Trg = readdata & (readdata ^ Cont);              //核心代码2
    Cont = readdata;                                //核心代码3

    return Trg;
}

uint8_t Key3_up(void){
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;

    uint8_t readdata = MY_KEY3;                     //按下是高电平,就不需要取反
    Trg = readdata & (readdata ^ Cont);              //核心代码2
    Cont = readdata;                                //核心代码3

    return Trg;
}


//注意此函数有响应优先级,KEY0>KEY1>KEY2>KEY3!!
uint8_t Key_Scan(void) {
    //printf("MY_KEY0:%d MY_KEY1:%d MY_KEY2:%d MY_KEY3:%d\n", MY_KEY0, MY_KEY1, MY_KEY2, MY_KEY3);
    if(GET_KEY_UNBOUNCE_FLAG()){
        SET_KEY_UNBOUNCE_FLAG(false);
        if (Key1_down())
            return MY_KEY_DOWN;
        else if (Key2_down())
            return MY_KEY_LEFT;
        else if (Key3_up())
            return MY_KEY_UP;
        else if (Key0_down())
            return MY_KEY_RIGHT;
    }
    return MY_KEY_NONE;// 无按键按下
}

void Key_Update(void)
{
    SET_KEY_UNBOUNCE_FLAG(true);
}
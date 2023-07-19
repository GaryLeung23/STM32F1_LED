#include "../Inc/key.h"

//���д�������    �������°�����ʱ��ֻ��һ��Trg=1.
static uint8_t Key1_down(bool is_debounce) {
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;
    uint8_t res;

    if(is_debounce){
        uint8_t readdata = MY_KEY1 ^ 1;                   //�����ǵ͵�ƽ,����Ҫȡ��
        Trg = readdata & (readdata ^ Cont);              //���Ĵ���2
        Cont = readdata;                                //���Ĵ���3

        res = Trg;
    }else{
        res = !MY_KEY1;
    }


    return res;
}

static uint8_t Key2_down(bool is_debounce) {
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;
    uint8_t res;

    if(is_debounce){
        uint8_t readdata = MY_KEY2 ^ 1;                   //�����ǵ͵�ƽ,����Ҫȡ��
        Trg = readdata & (readdata ^ Cont);              //���Ĵ���2
        Cont = readdata;                                //���Ĵ���3

        res = Trg;
    }else{
        res = !MY_KEY2;
    }


    return res;
}

static uint8_t Key0_down(bool is_debounce) {
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;
    uint8_t res;

    if(is_debounce){
        uint8_t readdata = MY_KEY0 ^ 1;                   //�����ǵ͵�ƽ,����Ҫȡ��
        Trg = readdata & (readdata ^ Cont);              //���Ĵ���2
        Cont = readdata;                                //���Ĵ���3

        res = Trg;
    }else{
        res = ! MY_KEY0;
    }


    return res;
}

uint8_t Key3_up(bool is_debounce) {
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;
    uint8_t res;

    if(is_debounce){
        uint8_t readdata = MY_KEY3;                   //�����Ǹߵ�ƽ,�Ͳ���Ҫȡ��
        Trg = readdata & (readdata ^ Cont);              //���Ĵ���2
        Cont = readdata;                                //���Ĵ���3

        res = Trg;
    }else{
        res = MY_KEY3;
    }


    return res;
}


//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>KEY3!!
uint8_t Key_Scan(void) {
    //printf("MY_KEY0:%d MY_KEY1:%d MY_KEY2:%d MY_KEY3:%d\n", MY_KEY0, MY_KEY1, MY_KEY2, MY_KEY3);
    if (Key1_down(true))
        return MY_KEY_DOWN;
    else if (Key2_down(false))
        return MY_KEY_LEFT;
    else if (Key3_up(true))
        return MY_KEY_UP;
    else if (Key0_down(false))
        return MY_KEY_RIGHT;

    return MY_KEY_NONE;// �ް�������
}

void SetKeyUserMode(key_user_mode_t mode){
    key_user_mode = mode;
}

key_user_mode_t GetKeyUserMode(void){
    return key_user_mode;
}
#include "../Inc/key.h"



//����������
//���ذ���ֵ
//continuous_mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��KEY0����
//2��KEY1����
//3��KEY2���� 
//4��KEY3���� WK_UP



//uint8_t Key_Scan_unbounce(bool continuous_mode) {
//    static uint8_t key_release = 1;//�������ɿ���־
//    if (continuous_mode)
//        key_release = 1;  //֧������
//    if (key_release && (MY_KEY0 == 0 || MY_KEY1 == 0 || MY_KEY2 == 0 || MY_KEY3 == 1)) {
//        key_delay_ms(5);//ȥ����
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
//    return 0;// �ް�������
//}


bool key_unbounce_flag = true;


//���д�������    �������°�����ʱ��ֻ��һ��Trg=1.
uint8_t Key1_down(void) {
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;

    uint8_t readdata = MY_KEY1 ^ 1;                   //�����ǵ͵�ƽ,����Ҫȡ��
    Trg = readdata & (readdata ^ Cont);              //���Ĵ���2
    Cont = readdata;                                //���Ĵ���3

    return Trg;
}
uint8_t Key2_down(void){
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;

    uint8_t readdata = MY_KEY2 ^ 1;                   //�����ǵ͵�ƽ,����Ҫȡ��
    Trg = readdata & (readdata ^ Cont);              //���Ĵ���2
    Cont = readdata;                                //���Ĵ���3

    return Trg;
}
uint8_t Key0_down(void){
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;

    uint8_t readdata = MY_KEY0 ^ 1;                  //�����ǵ͵�ƽ,����Ҫȡ��
    Trg = readdata & (readdata ^ Cont);              //���Ĵ���2
    Cont = readdata;                                //���Ĵ���3

    return Trg;
}

uint8_t Key3_up(void){
    static uint8_t Cont = 0;
    static uint8_t Trg = 0;

    uint8_t readdata = MY_KEY3;                     //�����Ǹߵ�ƽ,�Ͳ���Ҫȡ��
    Trg = readdata & (readdata ^ Cont);              //���Ĵ���2
    Cont = readdata;                                //���Ĵ���3

    return Trg;
}


//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>KEY3!!
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
    return MY_KEY_NONE;// �ް�������
}

void Key_Update(void)
{
    SET_KEY_UNBOUNCE_FLAG(true);
}
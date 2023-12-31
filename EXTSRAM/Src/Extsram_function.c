/*  Copyright (s) 2019 深圳百问网科技有限公司
 *  All rights reserved
 *
 * 文件名称：driver_sram.c
 * 摘要：
 *
 * 修改历史     版本号        Author       修改内容
 *--------------------------------------------------
 * 2020.6.6      v01        百问科技      创建文件
 *--------------------------------------------------
*/

#include "main.h"
#include "../Inc/Extsram_function.h"


/*
 *  函数名：void SRAM_WriteBufferWord(uint32_t *pdata, uint32_t addr, uint32_t sz)
 *  输入参数：pdata->要写入数据的首地址
 *            addr ->要写到RAM的初始地址
 *            sz   ->要写入的数据个数
 *  输出参数：无
 *  返回值：无
 *  函数作用：往片外RAM地址addr开始写入sz个字
*/
void SRAM_WriteBufferWord(uint32_t *pdata, uint32_t addr, uint32_t sz) {
    uint32_t i = 0;
    __IO uint32_t offset = (uint32_t) addr;

    for (i = 0; i < sz; i++) {
        *(__IO uint32_t *) (IS62WV51216_BASE_ADDR + offset) = pdata[i];
        offset += 4;;
    }
}

/*
 *  函数名：void SRAM_ReadBufferWord(uint32_t *pdata, uint32_t addr, uint32_t sz)
 *  输入参数：pdata->要读出数据的首地址
 *            addr ->要读出的RAM的初始地址
 *            sz   ->要写读出的数据个数
 *  输出参数：无
 *  返回值：无
 *  函数作用：往片外RAM地址addr开始读出sz个字
*/
void SRAM_ReadBufferWord(uint32_t *pdata, uint32_t addr, uint32_t sz) {
    uint16_t i = 0;
    __IO uint32_t offset = (uint32_t) addr;

    for (i = 0; i < sz; i++) {
        pdata[i] = *(__IO uint32_t *) (IS62WV51216_BASE_ADDR + offset);
        offset += 4;
    }
}

/*
 *  函数名：void SRAM_WriteBufferWord(uint16_t *pdata, uint32_t addr, uint16_t sz)
 *  输入参数：pdata->要写入数据的首地址
 *            addr ->要写到RAM的初始地址
 *            sz   ->要写入的数据个数
 *  输出参数：无
 *  返回值：无
 *  函数作用：往片外RAM地址addr开始写入sz个半字
*/
void SRAM_WriteBufferHalfWord(uint16_t *pdata, uint32_t addr, uint16_t sz) {
    uint32_t i = 0;
    __IO uint16_t offset = (uint16_t) addr;

    for (i = 0; i < sz; i++) {
        *(__IO uint16_t *) (IS62WV51216_BASE_ADDR + offset) = pdata[i];
        offset += 2;
    }
}

/*
 *  函数名：void SRAM_ReadBufferHalfWord(uint16_t *pdata, uint32_t addr, uint16_t sz)
 *  输入参数：pdata->要读出数据的首地址
 *            addr ->要读出的RAM的初始地址
 *            sz   ->要写读出的数据个数
 *  输出参数：无
 *  返回值：无
 *  函数作用：往片外RAM地址addr开始读出sz个半字
*/
void SRAM_ReadBufferHalfWord(uint16_t *pdata, uint32_t addr, uint16_t sz) {
    uint16_t i = 0;
    __IO uint16_t offset = (uint16_t) addr;

    for (i = 0; i < sz; i++) {
        pdata[i] = *(__IO uint16_t *) (IS62WV51216_BASE_ADDR + offset);
        offset += 2;
    }
}

/*
 *  函数名：void SRAM_WriteBufferBytes(uint8_t* pdata, uint32_t addr, uint16_t sz)
 *  输入参数：pdata->要写入数据的首地址
 *            addr ->要写到RAM的初始地址
 *            sz   ->要写入的数据个数
 *  输出参数：无
 *  返回值：无
 *  函数作用：往片外RAM地址addr开始写入sz个字节
*/
void SRAM_WriteBufferBytes(uint8_t *pdata, uint32_t addr, uint16_t sz) {
    uint32_t i = 0;
    __IO uint8_t offset = (uint8_t) addr;

    for (i = 0; i < sz; i++) {
        *(__IO uint8_t *) (IS62WV51216_BASE_ADDR + offset) = pdata[i];
        offset++;
    }
}

/*
 *  函数名：void SRAM_ReadBufferBytes(uint8_t* pdata, uint32_t addr, uint16_t sz)
 *  输入参数：pdata->要读出数据的首地址
 *            addr ->要读出的RAM的初始地址
 *            sz   ->要写读出的数据个数
 *  输出参数：无
 *  返回值：无
 *  函数作用：往片外RAM地址addr开始读出sz个字节
*/
void SRAM_ReadBufferBytes(uint8_t *pdata, uint32_t addr, uint16_t sz) {
    uint16_t i = 0;
    __IO uint8_t offset = (uint8_t) addr;

    for (i = 0; i < sz; i++) {
        pdata[i] = *(__IO uint8_t *) (IS62WV51216_BASE_ADDR + offset);
        offset++;
    }
}

uint8_t ExtSRAM_test(void) {
    uint8_t wbuf[32];
    uint8_t rbuf[32];
    int8_t res = 0;
    for (int i = 0; i < sizeof(wbuf); i++) {
        wbuf[i] = i;
    }
    SRAM_WriteBufferBytes(wbuf, 0xF0000, sizeof(wbuf));

    SRAM_ReadBufferBytes(rbuf, 0xF0000, sizeof(wbuf));

    for (int i = 0; i < sizeof(wbuf); i++) {
        if (rbuf[i] != wbuf[i]) {
            res = -1;
            break;
        }
    }


    return res;
}

/* Copy the data segment initializers from flash to EXTSRAM */
extern char _siextsram[];
extern char _sext_sram[];
extern char _eext_sram[];

void CopyFlashDataToExtSRAM(void) {
    uint32_t org;
    uint32_t des;
    org = (uint32_t) _siextsram;
    des = (uint32_t) _sext_sram - IS62WV51216_BASE_ADDR;
    SRAM_WriteBufferWord((uint32_t *) org, des, (_eext_sram - _sext_sram) / 4);
}
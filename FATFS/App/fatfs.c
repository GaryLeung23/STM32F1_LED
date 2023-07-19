/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */
#include "rtc.h"

FRESULT f_res;                  /* 文件操作结果 */
UINT fnum;                      /* 文件成功读写数量 */
BYTE ReadBuffer[1024] = {0};    /* 读缓冲区 */
const BYTE WriteBuffer[] =  "This is STM32 working with FatFs with the STM32CUBEMX tool.\r\n";          /* 写缓冲区 */
const TCHAR FileName[] = "0:/my_folder/FatFs_STM32.txt";
const TCHAR DirName[] = "0:/my_folder";
/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */



  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
    /* 获取系统时间并存入变量中 */
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    //printf("%d-%d-%d %d:%d:%d\r\n", sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds);
    /* 将FATFS时间结构体格式化 */
    //
    DWORD fatfs_time = ((DWORD) (sDate.Year +20) << 25) // Year = 1980 + sDate.Year，这里的20是因为RTC驱动中的年份是从2000年开始计算，而这里需�?1980�?始计�?
                       | ((DWORD) sDate.Month << 21)    // Month = sDate.Month
                       | ((DWORD) sDate.Date << 16)     // Day = sDate.Date
                       | ((DWORD) sTime.Hours << 11)    // Hour = sTime.Hours
                       | ((DWORD) sTime.Minutes << 5)   // Min = sTime.Minutes
                       | ((DWORD) sTime.Seconds >> 1);  //Sec = sTime.Seconds / 2，FAT的时间精度为2s
    return fatfs_time;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */
void MX_FATFS_TEST(void){
    printf("\r\n****** FatFs Example ******\r\n\r\n");

    //在外�????????? SD 卡挂载文件系统，文件系统挂载时会�????????? SD 卡初始化
    f_res = f_mount(&SDFatFS, (const TCHAR*)SDPath, 1);
    printf("\r\n****** Register the file system object to the FatFs module ******\r\n");
    /* 如果没有文件系统就格式化创建创建文件系统 */
    if(f_res == FR_NO_FILESYSTEM)
    {
        printf("The SD card does not yet have a file system and is about to be formatted...\r\n");
        /* 格式�????????? */
        f_res = f_mkfs((const TCHAR*)SDPath, 0, 0);
        if(f_res == FR_OK)
        {
            printf("The SD card successfully formatted the file system\r\n");
            /* 格式化后，先取消挂载 */
            f_res = f_mount(NULL, (const TCHAR*)SDPath, 1);
            /* 重新挂载 */
            f_res = f_mount(&SDFatFS, (const TCHAR*)SDPath, 1);
        }
        else
        {
            printf("The format failed\r\n");
            return;
        }
    }
    else if(f_res != FR_OK)
    {
        printf(" mount error : %d \r\n", f_res);
        return;
    }
    else
    {
        printf(" mount sucess!!! \r\n");
    }

    printf("\r\n****** Create new dir ******\r\n");

    /* 创建文件�?? */
    f_res = f_mkdir(DirName);

    if (f_res != FR_OK && f_res != FR_EXIST) {
        /* 文件夹创建失败，进行错误处理 */
        printf("Create directory failed: %d\r\n", f_res);
        return ;
    }

    /*----------------------- 文件系统测试：写测试 -----------------------------*/
    /* 打开文件，如果文件不存在则创建它 */
    printf("\r\n****** Create and Open new text file objects with write access ******\r\n");
    f_res = f_open(&SDFile, FileName, FA_CREATE_ALWAYS | FA_WRITE);
    if(f_res == FR_OK)
    {
        printf(" open file sucess!!! \r\n");
        /* 将指定存储区内容写入到文件内 */
        printf("\r\n****** Write data to the text files ******\r\n");
        f_res = f_write(&SDFile, WriteBuffer, sizeof(WriteBuffer), &fnum);
        if(f_res == FR_OK)
        {
            printf(" write file sucess!!! (%d)\n", fnum);
            printf(" write Data : %s\r\n", WriteBuffer);
        }
        else
        {
            printf(" write file error : %d\r\n", f_res);
            return;
        }
        /* 不再读写，关闭文�????????? */
        f_close(&SDFile);
    }
    else
    {
        printf(" open file error : %d\r\n", f_res);
        return;
    }

    /*------------------- 文件系统测试：读测试 ------------------------------------*/
    printf("\r\n****** Read data from the text files ******\r\n");
    f_res = f_open(&SDFile, FileName, FA_OPEN_EXISTING | FA_READ);
    if(f_res == FR_OK)
    {
        printf(" open file sucess!!! \r\n");
        f_res = f_read(&SDFile, ReadBuffer, sizeof(ReadBuffer), &fnum);
        if(f_res == FR_OK)
        {
            printf("read sucess!!! (%d)\n", fnum);
            printf("read Data : %s\r\n", ReadBuffer);
        }
        else
        {
            printf(" read error!!! %d\r\n", f_res);
            return;
        }
    }
    else
    {
        printf(" open file error : %d\r\n", f_res);
        return;
    }
    /* 不再读写，关闭文�????????? */
    f_close(&SDFile);

    DIR dir;    /* 文件夹句�?? */
    FILINFO fno;
    printf("\r\n****** Read Dir info ******\r\n");
    /* 打开文件�?? */
    f_res = f_opendir(&dir, DirName);

    if (f_res != FR_OK) {
        /* 文件夹打�??失败，进行错误处�?? */
        printf("\n****** Open directory failed: %d\r\n", f_res);
    }

/* 文件夹打�??成功，可以读取文件夹中的文件 */
    while (1) {
        /* 读取文件夹中的文件信�?? */
        f_res = f_readdir(&dir, &fno);

        if (f_res != FR_OK || fno.fname[0] == 0) {
            /* 读取文件夹失败或者文件夹已经读取完毕 */
            //printf(" failed or completed res:%d,fno.fname=%s\r\n",f_res,fno.fname);
            break;
        }

        if (fno.fattrib & AM_DIR) {
            /* 如果是子目录 */
            printf("DIR: %s\r\n", fno.fname);
        } else {
            /* 如果是文�?? */
            printf("FILE: %s\r\n", fno.fname);
        }
    }

/* 关闭文件�?? */
    f_closedir(&dir);






    /* 不再使用文件系统，取消挂载文件系�????????? */
    f_mount(NULL,(const TCHAR*)SDPath , 1);


}

/* USER CODE END Application */

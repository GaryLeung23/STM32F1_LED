#ifndef __LCD_FUNCTION_H
#define __LCD_FUNCTION_H

#include "main.h"
#include "tim.h"

/********************GPIOD************************/
/* PD0 PD1 PD4 PD5 PD8 PD9 PD10 PD14 PD15 */
/*********************GPIOG***********************/
#define LCD_GPIOD_PIN           ( GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 \
                            | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15)

/********************GPIOE************************/
/* PE2 PE7 PE8 PE9 PE10 PE11 PE12 PE13 PE14 PE15 */
/*********************GPIOG***********************/
#define LCD_GPIOE_PIN           ( GPIO_PIN_2 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 \
                            | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)


/********************FSMC_D***********************/
#define LCD_D0_PIN                  GPIO_PIN_14 // PD14
#define LCD_D1_PIN                  GPIO_PIN_15 // PD15
#define LCD_D2_PIN                  GPIO_PIN_0  // PD0
#define LCD_D3_PIN                  GPIO_PIN_1  // PD1
#define LCD_D4_PIN                  GPIO_PIN_7  // PE7
#define LCD_D5_PIN                  GPIO_PIN_8  // PE8
#define LCD_D6_PIN                  GPIO_PIN_9  // PE9
#define LCD_D7_PIN                  GPIO_PIN_10 // PE10
#define LCD_D8_PIN                  GPIO_PIN_11 // PE11
#define LCD_D9_PIN                  GPIO_PIN_12 // PE12
#define LCD_D10_PIN                 GPIO_PIN_13 // PE13
#define LCD_D11_PIN                 GPIO_PIN_14 // PE14
#define LCD_D12_PIN                 GPIO_PIN_15 // PE15
#define LCD_D13_PIN                 GPIO_PIN_8  // PD8
#define LCD_D14_PIN                 GPIO_PIN_9  // PD9
#define LCD_D15_PIN                 GPIO_PIN_10 // PD10

/********************其他信号线****  *******************/
#define LCD_CS_PIN                  GPIO_PIN_12  // PG12 NE4
#define LCD_RS_PIN                  GPIO_PIN_0   // PG0  A10
#define LCD_WR_PIN                  GPIO_PIN_5   // PD5  NWE
#define LCD_RD_PIN                  GPIO_PIN_4   // PD4  OE
#define LCD_BL_PIN                  GPIO_PIN_0   // PB0  BL

//#define LCD_RST_PIN                 GPIO_PIN_11  // 连接到芯片RST,所以不需要

#define GPIOB_CLK_EN()              __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIOC_CLK_EN()              __HAL_RCC_GPIOC_CLK_ENABLE()
#define GPIOD_CLK_EN()              __HAL_RCC_GPIOD_CLK_ENABLE()
#define GPIOE_CLK_EN()              __HAL_RCC_GPIOE_CLK_ENABLE()
#define GPIOF_CLK_EN()              __HAL_RCC_GPIOF_CLK_ENABLE()
#define GPIOG_CLK_EN()              __HAL_RCC_GPIOG_CLK_ENABLE()

#define LCD_BL(VALUE)               HAL_GPIO_WritePin(GPIOB, LCD_BL_PIN, VALUE)
#define LCD_CS(VALUE)               HAL_GPIO_WritePin(GPIOG, LCD_CS_PIN, VALUE)
#define LCD_RS(VALUE)               HAL_GPIO_WritePin(GPIOG, LCD_RS_PIN, VALUE)
#define LCD_WR(VALUE)               HAL_GPIO_WritePin(GPIOD, LCD_WR_PIN, VALUE)
#define LCD_RD(VALUE)               HAL_GPIO_WritePin(GPIOD, LCD_RD_PIN, VALUE)
//#define LCD_RST(VALUE)              HAL_GPIO_WritePin(GPIOF, LCD_RST_PIN, VALUE)


/*************** LCD延时函数重定义 ***************/
#define lcd_delay_ms(m)             HAL_Delay(m)
#define lcd_delay_us(u)             us_timer_delay(u)

/*************** LCD 常用颜色定义 ***************/
#define WHITE             0xFFFF
#define BLACK             0x0000
#define RED             0xF800
#define GREEN             0x07E0
#define BLUE             0x001F
#define YELLOW             0xFFE0

/*************** LCD 扫描方向定义 ***************/
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 

/*************** LCD 结构体定义 ***************/
typedef struct {
    __IO uint16_t reg;   // 寄存器值
    __IO uint16_t data;  // 数据值
} _lcd;   /* LCD 指令结构体，用于读写LCD数据和寄存器 */

typedef struct {
    uint16_t dev_id;    // 设备ID
    uint16_t hor_res;   // LCD水平显示长度
    uint16_t ver_res;   // LCD垂直显示长度
    uint16_t scan_mode; // 扫描模式

    uint16_t gram_cmd;  // 写入GRAM的指令
    uint16_t setx_cmd;  // 设置X坐标的指令
    uint16_t sety_cmd;  // 设置Y坐标的指令
} _lcddev;   /* LCD 设备信息 */

typedef struct {
    uint16_t backcolor; // 背景颜色值
    uint16_t textcolor; // 笔迹颜色值
} _lcd_color;    /* LCD 显示颜色结构体，用于设置背景颜色和笔迹颜色 */

/********** 全局变量声明 **********/
extern _lcddev lcddev;
extern _lcd_color lcd_color;


/*************** 读写API声明 ***************/

/* 函数名：  void LCD_Write_Cmd(uint16_t _cmd)
 * 输入参数：_cmd->要写入的指令
 * 输出参数：无
 * 返回值：  无
 * 函数作用：往LCD写入指令
*/
extern void LCD_Write_Cmd(uint16_t _cmd);

/* 函数名：  void LCD_Write_Data(uint16_t _data)
 * 输入参数：_data->要写入的数据
 * 输出参数：无
 * 返回值：  无
 * 函数作用：往LCD写入数据
*/
extern void LCD_Write_Data(uint16_t _data);

/* 函数名：  uint16_t LCD_Read_Data(void)
 * 输入参数：无
 * 输出参数：无
 * 返回值：  返回寄存器的值
 * 函数作用：返回寄存器的值
*/
extern uint16_t LCD_Read_Data(void);

/* 函数名：  void LC_Write_Reg(uint16_t _cmd, uint16_t _data)
 * 输入参数：_cmd->要写入的指令；_data->要写入的数据
 * 输出参数：无
 * 返回值：  无
 * 函数作用：
*/
extern void LCD_Write_Reg(uint16_t _cmd, uint16_t _data);

/*************** 颜色设置API声明 ***************/
extern void LCD_SetBackColor(uint16_t _backcolor);

extern uint16_t LCD_GetBackColor(void);

extern void LCD_SetTextColor(uint16_t _textcolor);

extern uint16_t LCD_GetTextColor(void);

extern void LCD_SetAllColor(uint16_t _backcolor, uint16_t _textcolor);

extern void LCD_GetAllColor(uint16_t *_backcolor, uint16_t *_textcolor);

/*************** LCD功能函数声明 ***************/
extern void LCD_Scan_Dir(uint8_t _dir);

extern void LCD_GRAM_Scan(uint8_t _opt);

extern void LCD_SetCursor(uint16_t x, uint16_t y);

extern void LCD_Clear(uint16_t _color);

extern void LCD_Fill(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);

extern void LCD_Color_Fill(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t *color);

extern uint32_t LCD_Pow(uint8_t m, uint8_t n);

extern void LCD_DrawPoint(uint16_t x, uint16_t y);

extern void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint16_t color);

extern void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode);

extern void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size);

extern void LCD_ShowxNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);

extern void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p);

extern void LCD_SetPointPixel(uint16_t usX, uint16_t usY);

extern void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

extern void LCD_DrawCross(uint16_t usX, uint16_t usY);

/********** LCD初始化函数声明 **********/
extern void LCD_Init(void);

#endif /* __LCD_FUNCTION_H */

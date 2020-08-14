#include "usmart.h"
#include "usmart_str.h"

/*********************************************************************************
			  ___   _     _____  _____  _   _  _____  _____  _   __
			 / _ \ | |   |_   _||  ___|| \ | ||_   _||  ___|| | / /
			/ /_\ \| |     | |  | |__  |  \| |  | |  | |__  | |/ /
			|  _  || |     | |  |  __| | . ` |  | |  |  __| |    \
			| | | || |_____| |_ | |___ | |\  |  | |  | |___ | |\  \
			\_| |_/\_____/\___/ \____/ \_| \_/  \_/  \____/ \_| \_/

 *	******************************************************************************
 *	本程序只供学习使用，未经作者许可，不得用于其它任何用途
 *	ALIENTEK Pandora STM32L475 IOT开发板
 *	USMART驱动代码
 *	正点原子@ALIENTEK
 *	技术论坛:www.openedv.com
 *	版本：V3.3
 *	版权所有，盗版必究。
 *	Copyright(C) 广州市星翼电子科技有限公司 2014-2024
 *	All rights reserved
 *	******************************************************************************/

////////////////////////////用户配置区///////////////////////////////////////////////
//这下面要包含所用到的函数所申明的头文件(用户自己添加)
#include "delay.h"
#include "sys.h"
#include "lcd.h"

//函数名列表初始化(用户自己添加)
//用户直接在这里输入要执行的函数名及其查找串
struct _m_usmart_nametab usmart_nametab[] =
{
#if USMART_USE_WRFUNS==1 	//如果使能了读写操作
    (void*)read_addr, "u32 read_addr(u32 addr)",
    (void*)write_addr, "void write_addr(u32 addr,u32 val)",
#endif
    (void*)delay_ms, "void delay_ms(u16 nms)",
    (void*)delay_us, "void delay_us(u32 nus)",
    (void*)LCD_Clear, "void LCD_Clear(u16 Color)",
    (void*)LCD_Fill, "void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)",
    (void*)LCD_DrawLine, "void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)",
    (void*)LCD_DrawRectangle, "void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)",
    (void*)LCD_Draw_Circle, "void Draw_Circle(u16 x0,u16 y0,u8 r)",
    (void*)LCD_ShowNum, "void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)",
    (void*)LCD_ShowString, "void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)",
    (void*)LCD_ShowxNum, "void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode)",
    (void*)Display_ALIENTEK_LOGO, "void Display_ALIENTEK_LOGO(u16 x,u16 y)",

};
///////////////////////////////////END///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//函数控制管理器初始化
//得到各个受控函数的名字
//得到函数总数量
struct _m_usmart_dev usmart_dev =
{
    usmart_nametab,
    usmart_init,
    usmart_cmd_rec,
    usmart_exe,
    usmart_scan,
    sizeof(usmart_nametab) / sizeof(struct _m_usmart_nametab), //函数数量
    0,	  	//参数数量
    0,	 	//函数ID
    1,		//参数显示类型,0,10进制;1,16进制
    0,		//参数类型.bitx:,0,数字;1,字符串
    0,	  	//每个参数的长度暂存表,需要MAX_PARM个0初始化
    0,		//函数的参数,需要PARM_LEN个0初始化
};




















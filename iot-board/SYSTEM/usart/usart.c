#include "usart.h"
#include "delay.h"
#include "stdio.h"	
#include "stdarg.h"	 
#include "string.h"
#include "timer.h"  
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
 *	串口1初始化
 *	正点原子@ALIENTEK
 *	技术论坛:www.openedv.com
 *	修改日期:2015/9/7
 *	版本：V1.5
 *	版权所有，盗版必究。
 *	Copyright(C) 广州市星翼电子科技有限公司 2009-2024
 *	All rights reserved
 *	******************************************************************************
 *	V1.0修改说明
 *	加入以下代码,支持printf函数,而不需要选择use MicroLIB
 *	******************************************************************************/


#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;
};

FILE __stdout;
/**
 * @brief	定义_sys_exit()以避免使用半主机模式
 *
 * @param	void
 *
 * @return  void
 */
void _sys_exit(int x)
{
    x = x;
}
/**
 * @brief	重定义fputc函数
 *
 * @param	ch		输出字符量
 * @param	f		文件指针
 *
 * @return  void
 */
int fputc(int ch, FILE *f)
{
    while((USART1->ISR & 0X40) == 0); //循环发送,直到发送完毕

    USART1->TDR = (u8) ch;
    return ch;
}
#endif


//#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲

u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.
u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//发送缓冲,最大USART2_MAX_SEND_LEN字节
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0;     //接收状态标记
u16 USART2_RX_STA=0;  

UART_HandleTypeDef UART2_Handler; //UART句柄
UART_HandleTypeDef UART1_Handler; //UART句柄


/**
 * @brief	初始化串口1函数
 *
 * @param	bound	串口波特率
 *
 * @return  void
 */
void uart_init(u32 bound)
{
    //UART 初始化设置
    UART1_Handler.Instance = USART1;					  //USART1
    UART1_Handler.Init.BaudRate = bound;				  //波特率
    UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B; //字长为8位数据格式
    UART1_Handler.Init.StopBits = UART_STOPBITS_1;	  //一个停止位
    UART1_Handler.Init.Parity = UART_PARITY_NONE;		  //无奇偶校验位
    UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE; //无硬件流控
    UART1_Handler.Init.Mode = UART_MODE_TX_RX;		  //收发模式
    HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()会使能UART1

    __HAL_UART_ENABLE_IT(&UART1_Handler, UART_IT_RXNE); //开启接收中断
			//抢占优先级3，子优先级3
}
void usart2_init(u32 bound)
{  	 
	
	//UART 初始化设置
	UART2_Handler.Instance=USART2;					    //USART1
	UART2_Handler.Init.BaudRate=bound;				    //波特率
	UART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART2_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
	UART2_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
	UART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	UART2_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	HAL_UART_Init(&UART2_Handler);					    //HAL_UART_Init()会使能UART2
	

}

/**
 * @brief	HAL库串口底层初始化，时钟使能，引脚配置，中断配置
 *
 * @param	huart	串口句柄
 *
 * @return  void
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_Initure;

    if(huart->Instance == USART1) //如果是串口1，进行串口1 MSP初始化
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();				//使能GPIOA时钟
        __HAL_RCC_USART1_CLK_ENABLE();				//使能USART1时钟

        GPIO_Initure.Pin = GPIO_PIN_9;				//PA9
        GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
        GPIO_Initure.Pull = GPIO_PULLUP;			//上拉
        GPIO_Initure.Speed = GPIO_SPEED_FAST;		//高速
        GPIO_Initure.Alternate = GPIO_AF7_USART1;	//复用为USART1
        HAL_GPIO_Init(GPIOA, &GPIO_Initure);	   	//初始化PA9

        GPIO_Initure.Pin = GPIO_PIN_10;				//PA10
        HAL_GPIO_Init(GPIOA, &GPIO_Initure);	   	//初始化PA10
    }
#if EN_USART1_RX   //如果使能了接收
		    HAL_NVIC_EnableIRQ(USART1_IRQn);					//使能USART1中断通道
			HAL_NVIC_SetPriority(USART1_IRQn, 3, 3);	
#endif
	 if(huart->Instance == USART2) //如果是串口1，进行串口1 MSP初始化
    {
		GPIO_InitTypeDef GPIO_Initure;
		
		__HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
		__HAL_RCC_USART2_CLK_ENABLE();			//使能USART1时钟
		
		GPIO_Initure.Pin=GPIO_PIN_2;			//PB10
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FAST;		//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART2;	//复用为USART1
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PB10

		GPIO_Initure.Pin=GPIO_PIN_3;			//PB11
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PB11
		
	//  __HAL_UART_DISABLE_IT(huart,UART_IT_TC);
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);		//开启接收中断
		HAL_NVIC_EnableIRQ(USART2_IRQn);				//使能USART2中断
		HAL_NVIC_SetPriority(USART2_IRQn,2,3);			//抢占优先级3，子优先级3	
		
		TIM3_Init(1000-1,8400-1);		//100ms中断
		USART2_RX_STA=0;		//清零
		TIM3->CR1&=~(1<<0);        //关闭定时器7
	
		
	}
}


void u2_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
		while((USART2->ISR&0X40)==0);			//循环发送,直到发送完毕   
		USART2->TDR=USART2_TX_BUF[j];  
	} 
}


void USART2_IRQHandler(void)
{
	u8 res;	      
	if(__HAL_UART_GET_FLAG(&UART2_Handler,UART_FLAG_RXNE)!=RESET)//接收到数据
	{	 
		HAL_UART_Receive(&UART2_Handler,&res,1,1000);
//		res=USART2->DR; 			 
		if((USART2_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//还可以接收数据
			{
//				__HAL_TIM_SetCounter(&TIM3_Handler,0);	
				TIM3->CNT=0;         				//计数器清空	
				if(USART2_RX_STA==0) 				//使能定时器7的中断 
				{
//					__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
					TIM3->CR1|=1<<0;     			//使能定时器7
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	}  				 											 
} 


/**
 * @brief	串口1中断服务程序
 *
 * @remark	下面代码我们直接把中断控制逻辑写在中断服务函数内部
 * 			说明：采用HAL库处理逻辑，效率不高。
 *
 * @param   void
 *
 * @return  void
 */
void USART1_IRQHandler(void)
{
    u8 Res;

    if((__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_RXNE) != RESET)) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    {
        HAL_UART_Receive(&UART1_Handler, &Res, 1, 1000);

        if((USART_RX_STA & 0x8000) == 0) //接收未完成
        {
            if(USART_RX_STA & 0x4000) //接收到了0x0d
            {
                if(Res != 0x0a)USART_RX_STA = 0; //接收错误,重新开始

                else USART_RX_STA |= 0x8000;	//接收完成了
            }
            else //还没收到0X0D
            {
                if(Res == 0x0d)USART_RX_STA |= 0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res ;
                    USART_RX_STA++;

                    if(USART_RX_STA > (USART_REC_LEN - 1))USART_RX_STA = 0; //接收数据错误,重新开始接收
                }
            }
        }
    }
    HAL_UART_IRQHandler(&UART1_Handler);
}

//#endif







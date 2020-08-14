#include "sys.h"
#include "usart2.h"	  
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"
#include "timer.h"  
//////////////////////////////////////////////////////////////////////////////////	   
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������ 
//����3 HAL�⺯����ʼ������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2016/3/14
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//�޸�˵��
//��
////////////////////////////////////////////////////////////////////////////////// 	


//���ڽ��ջ�����

u8 aRxBuffer[RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//���ջ���,���USART2_MAX_RECV_LEN���ֽ�.
u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�

//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
u16 USART2_RX_STA=0;  
UART_HandleTypeDef UART2_Handler; //UART���
//TIM_HandleTypeDef TIM3_Handler;   //TIM���
  
//��ʼ��IO ����3
//bound:������ 
void USART2_init(u32 bound)
{  	 
	
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
	__HAL_RCC_USART2_CLK_ENABLE();			//ʹ��USART1ʱ��
	
	GPIO_Initure.Pin=GPIO_PIN_2;			//PB10
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
	GPIO_Initure.Pull=GPIO_PULLUP;			//����
	GPIO_Initure.Speed=GPIO_SPEED_FAST;		//����
	GPIO_Initure.Alternate=GPIO_AF7_USART2;	//����ΪUSART1
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PB10

	GPIO_Initure.Pin=GPIO_PIN_3;			//PB11
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PB11
	
	
	//UART ��ʼ������
	UART2_Handler.Instance=USART2;					    //USART1
	UART2_Handler.Init.BaudRate=bound;				    //������
	UART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	UART2_Handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	UART2_Handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	UART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	UART2_Handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	HAL_UART_Init(&UART2_Handler);					    //HAL_UART_Init()��ʹ��UART2
	

}
//UART�ײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
//�˺����ᱻHAL_UART_Init()����
//huart:���ھ��
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
	__HAL_RCC_USART2_CLK_ENABLE();			//ʹ��USART1ʱ��
	
	GPIO_Initure.Pin=GPIO_PIN_2;			//PB10
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
	GPIO_Initure.Pull=GPIO_PULLUP;			//����
	GPIO_Initure.Speed=GPIO_SPEED_FAST;		//����
	GPIO_Initure.Alternate=GPIO_AF7_USART2;	//����ΪUSART1
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PB10

	GPIO_Initure.Pin=GPIO_PIN_3;			//PB11
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PB11
	
//  __HAL_UART_DISABLE_IT(huart,UART_IT_TC);
	__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);		//���������ж�
	HAL_NVIC_EnableIRQ(USART2_IRQn);				//ʹ��USART2�ж�
	HAL_NVIC_SetPriority(USART2_IRQn,2,3);			//��ռ���ȼ�3�������ȼ�3	
	
	TIM3_Init(1000-1,8400-1);		//100ms�ж�
	USART2_RX_STA=0;		//����
	TIM3->CR1&=~(1<<0);        //�رն�ʱ��7
	
}
//����2,printf ����
//ȷ��һ�η������ݲ�����USART2_MAX_SEND_LEN�ֽ�
void u3_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		//�˴η������ݵĳ���
	for(j=0;j<i;j++)							//ѭ����������
	{
		while((USART2->ISR&0X40)==0);			//ѭ������,ֱ���������   
		USART2->TDR=USART2_TX_BUF[j];  
	} 
}


void USART2_IRQHandler(void)
{
	u8 res;	      
	if(__HAL_UART_GET_FLAG(&UART2_Handler,UART_FLAG_RXNE)!=RESET)//���յ�����
	{	 
		HAL_UART_Receive(&UART2_Handler,&res,1,1000);
//		res=USART2->DR; 			 
		if((USART2_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//�����Խ�������
			{
//				__HAL_TIM_SetCounter(&TIM3_Handler,0);	
				TIM3->CNT=0;         				//���������	
				if(USART2_RX_STA==0) 				//ʹ�ܶ�ʱ��7���ж� 
				{
//					__HAL_RCC_TIM3_CLK_ENABLE();            //ʹ��TIM3ʱ��
					TIM3->CR1|=1<<0;     			//ʹ�ܶ�ʱ��7
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//��¼���յ���ֵ	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
			} 
		}
	}  				 											 
} 

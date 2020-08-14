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
 *	������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
 *	ALIENTEK Pandora STM32L475 IOT������
 *	����1��ʼ��
 *	����ԭ��@ALIENTEK
 *	������̳:www.openedv.com
 *	�޸�����:2015/9/7
 *	�汾��V1.5
 *	��Ȩ���У�����ؾ���
 *	Copyright(C) ������������ӿƼ����޹�˾ 2009-2024
 *	All rights reserved
 *	******************************************************************************
 *	V1.0�޸�˵��
 *	�������´���,֧��printf����,������Ҫѡ��use MicroLIB
 *	******************************************************************************/


#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
    int handle;
};

FILE __stdout;
/**
 * @brief	����_sys_exit()�Ա���ʹ�ð�����ģʽ
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
 * @brief	�ض���fputc����
 *
 * @param	ch		����ַ���
 * @param	f		�ļ�ָ��
 *
 * @return  void
 */
int fputc(int ch, FILE *f)
{
    while((USART1->ISR & 0X40) == 0); //ѭ������,ֱ���������

    USART1->TDR = (u8) ch;
    return ch;
}
#endif


//#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.

u8 aRxBuffer[RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���

u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//���ջ���,���USART2_MAX_RECV_LEN���ֽ�.
u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA = 0;     //����״̬���
u16 USART2_RX_STA=0;  

UART_HandleTypeDef UART2_Handler; //UART���
UART_HandleTypeDef UART1_Handler; //UART���


/**
 * @brief	��ʼ������1����
 *
 * @param	bound	���ڲ�����
 *
 * @return  void
 */
void uart_init(u32 bound)
{
    //UART ��ʼ������
    UART1_Handler.Instance = USART1;					  //USART1
    UART1_Handler.Init.BaudRate = bound;				  //������
    UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B; //�ֳ�Ϊ8λ���ݸ�ʽ
    UART1_Handler.Init.StopBits = UART_STOPBITS_1;	  //һ��ֹͣλ
    UART1_Handler.Init.Parity = UART_PARITY_NONE;		  //����żУ��λ
    UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE; //��Ӳ������
    UART1_Handler.Init.Mode = UART_MODE_TX_RX;		  //�շ�ģʽ
    HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()��ʹ��UART1

    __HAL_UART_ENABLE_IT(&UART1_Handler, UART_IT_RXNE); //���������ж�
			//��ռ���ȼ�3�������ȼ�3
}
void usart2_init(u32 bound)
{  	 
	
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

/**
 * @brief	HAL�⴮�ڵײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
 *
 * @param	huart	���ھ��
 *
 * @return  void
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_Initure;

    if(huart->Instance == USART1) //����Ǵ���1�����д���1 MSP��ʼ��
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();				//ʹ��GPIOAʱ��
        __HAL_RCC_USART1_CLK_ENABLE();				//ʹ��USART1ʱ��

        GPIO_Initure.Pin = GPIO_PIN_9;				//PA9
        GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//�����������
        GPIO_Initure.Pull = GPIO_PULLUP;			//����
        GPIO_Initure.Speed = GPIO_SPEED_FAST;		//����
        GPIO_Initure.Alternate = GPIO_AF7_USART1;	//����ΪUSART1
        HAL_GPIO_Init(GPIOA, &GPIO_Initure);	   	//��ʼ��PA9

        GPIO_Initure.Pin = GPIO_PIN_10;				//PA10
        HAL_GPIO_Init(GPIOA, &GPIO_Initure);	   	//��ʼ��PA10
    }
#if EN_USART1_RX   //���ʹ���˽���
		    HAL_NVIC_EnableIRQ(USART1_IRQn);					//ʹ��USART1�ж�ͨ��
			HAL_NVIC_SetPriority(USART1_IRQn, 3, 3);	
#endif
	 if(huart->Instance == USART2) //����Ǵ���1�����д���1 MSP��ʼ��
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
		
	//  __HAL_UART_DISABLE_IT(huart,UART_IT_TC);
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);		//���������ж�
		HAL_NVIC_EnableIRQ(USART2_IRQn);				//ʹ��USART2�ж�
		HAL_NVIC_SetPriority(USART2_IRQn,2,3);			//��ռ���ȼ�3�������ȼ�3	
		
		TIM3_Init(1000-1,8400-1);		//100ms�ж�
		USART2_RX_STA=0;		//����
		TIM3->CR1&=~(1<<0);        //�رն�ʱ��7
	
		
	}
}


void u2_printf(char* fmt,...)  
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


/**
 * @brief	����1�жϷ������
 *
 * @remark	�����������ֱ�Ӱ��жϿ����߼�д���жϷ������ڲ�
 * 			˵��������HAL�⴦���߼���Ч�ʲ��ߡ�
 *
 * @param   void
 *
 * @return  void
 */
void USART1_IRQHandler(void)
{
    u8 Res;

    if((__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_RXNE) != RESET)) //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
    {
        HAL_UART_Receive(&UART1_Handler, &Res, 1, 1000);

        if((USART_RX_STA & 0x8000) == 0) //����δ���
        {
            if(USART_RX_STA & 0x4000) //���յ���0x0d
            {
                if(Res != 0x0a)USART_RX_STA = 0; //���մ���,���¿�ʼ

                else USART_RX_STA |= 0x8000;	//���������
            }
            else //��û�յ�0X0D
            {
                if(Res == 0x0d)USART_RX_STA |= 0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res ;
                    USART_RX_STA++;

                    if(USART_RX_STA > (USART_REC_LEN - 1))USART_RX_STA = 0; //�������ݴ���,���¿�ʼ����
                }
            }
        }
    }
    HAL_UART_IRQHandler(&UART1_Handler);
}

//#endif







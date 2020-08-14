#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "w25qxx.h"
#include "sd_card.h"
#include "beep.h"
#include "ff.h"
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"
#include "usmart.h"
#include "common.h"
#include "aht10.h"
#define tempThreshold 35
/*********************************************************************************
			  ___   _     _____  _____  _   _  _____  _____  _   __
			 / _ \ | |   |_   _||  ___|| \ | ||_   _||  ___|| | / /
			/ /_\ \| |     | |  | |__  |  \| |  | |  | |__  | |/ /
			|  _  || |     | |  |  __| | . ` |  | |  |  __| |    \
			| | | || |_____| |_ | |___ | |\  |  | |  | |___ | |\  \
			\_| |_/\_____/\___/ \____/ \_| \_/  \_/  \____/ \_| \_/

 *	******************************************************************************
 *	����ԭ�� Pandora STM32L475 IoT������	ʵ��15
 *	ADCģ��ת��ʵ��		HAL��汾
 *	����֧�֣�www.openedv.com
 *	�Ա����̣�http://openedv.taobao.com
 *	��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 *	������������ӿƼ����޹�˾
 *	���ߣ�����ԭ�� @ALIENTEK
 *	******************************************************************************/

int main(void)
{
	 u8 t=0,tt=0;
		u8 key=0;
		u8 port[4]="8080";
    float temperature, humidity;
    HAL_Init();
    SystemClock_Config();		//��ʼ��ϵͳʱ��Ϊ80M
    delay_init(80); 			//��ʼ����ʱ����    80Mϵͳʱ��
    uart_init(115200);			//��ʼ�����ڣ�������Ϊ115200
	usart2_init(115200);
    LED_Init();					//��ʼ��LED
    LCD_Init();					//��ʼ��LCD
	KEY_Init();					//��ʼ������
	W25QXX_Init();				//��ʼ��W25Q256
	usmart_dev.init(80);		//��ʼ�����ڵ������
    my_mem_init(SRAM1);			//��ʼ���ڲ�SRAM1�ڴ��
    my_mem_init(SRAM2);			//��ʼ���ڲ�SRAM2�ڴ��
		BEEP_Init();
    exfuns_init();		            //Ϊfatfs��ر��������ڴ�
//    f_mount(fs[0], "0:", 1);        //����SD��
    f_mount(fs[1], "1:", 1);        //����SPI FLASH.
	
	
	while(font_init()) 		        //����ֿ�
    {
        LCD_Clear(WHITE);		   	//����
        POINT_COLOR = RED;			//��������Ϊ��ɫ
        Display_ALIENTEK_LOGO(0, 0);
        LCD_ShowString(30, 100, 200, 16, 16, "Pandora STM32L4 IOT");
        while(SD_Init())			//���SD��
        {
            LCD_ShowString(30, 120, 200, 16, 16, "SD Card Failed!");
            delay_ms(200);
            LCD_Fill(30, 120, 200 + 30, 120 + 16, WHITE);
            delay_ms(200);
        }
        LCD_ShowString(30, 120, 200, 16, 16, "SD Card OK");
        LCD_ShowString(30, 140, 200, 16, 16, "Font Updating...");
        key = update_font(30, 160, 16, (u8 *)"0:"); //�����ֿ�
        while(key)//����ʧ��
        {
            LCD_ShowString(30, 160, 200, 16, 16, "Font Update Failed!");
            delay_ms(200);
            LCD_Fill(30, 160, 200 + 20, 160 + 16, WHITE);
            delay_ms(200);
        }
        LCD_ShowString(30, 160, 200, 16, 16, "Font Update Success!            ");
        delay_ms(1500);
        LCD_Clear(WHITE);//����
    }
	
	
	
    POINT_COLOR = RED;
	u8 timex=0; 
	u8 ipbuf[16]; 	//IP����
	u8 *p;
	u16 t1=999;		//���ٵ�һ�λ�ȡ����״̬
	u8 res=0;
	u16 rlen=0;
	u8 constate=0;	//����״̬
	u8 sramx = 0;
	p=mymalloc(sramx,32);							//����32�ֽ��ڴ�
	atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//����WIFI STAģʽ
	atk_8266_send_cmd("AT+RST","OK",20);		//DHCP�������ر�(��APģʽ��Ч) 
	delay_ms(1000);         //��ʱ3S�ȴ������ɹ�
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",wifista_ssid,wifista_password);//�������߲���:ssid,����
	while(atk_8266_send_cmd(p,"WIFI GOT IP",300));					//����Ŀ��·����,���һ��IP

			LCD_Clear(WHITE);
			POINT_COLOR=RED;
			if(atk_8266_ip_set("WIFI-STA Զ��IP����",(u8*)ATK_ESP8266_WORKMODE_TBL[1],(u8*)portnum,ipbuf))//goto PRESTA;	//IP����
			atk_8266_send_cmd("AT+CIPMUX=0","OK",20);   //0�������ӣ�1��������
			printf("%s",ipbuf);
			sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //����Ŀ��TCP������
			while(atk_8266_send_cmd(p,"OK",200))
			{
					LCD_Clear(WHITE);
					POINT_COLOR=RED;
					Show_Str_Mid(0,20,"WK_UP:������ѡ",16,240);
					Show_Str(30,60,200,12,"ATK-ESP ����TCPʧ��",12); //����ʧ��	 
					key=KEY_Scan(0);
				//	if(key==WKUP_PRES)goto PRESTA;
			}	
			atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //����ģʽΪ��͸��			

			LCD_Clear(WHITE);
			POINT_COLOR=RED;
			//Show_Str_Mid(0,10,"ATK-ESP WIFI-STA ����",16,240);
			//Show_Str(30,30,200,16,"��������ATK-ESPģ��,���Ե�...",12);			
			//LCD_Fill(30,30,239,50+12,WHITE);			//���֮ǰ����ʾ
			//Show_Str(30,30,200,16,"WK_UP:�˳�����  KEY0:��������",12);
			//LCD_Fill(30,60,239,80+12,WHITE);
			atk_8266_get_wanip(ipbuf);//������ģʽ,��ȡWAN IP
			sprintf((char*)p,"IP��ַ:%s �˿�:%s",ipbuf,(u8*)portnum);
			Show_Str(30,45,200,12,p,12);				//��ʾIP��ַ�Ͷ˿�	
			Show_Str(30,60,200,12,"״̬:",12); 		//����״̬
			//Show_Str(120,60,200,12,"ģʽ:",12); 		//����״̬
			Show_Str(30,80,200,12,"��������:",12); 	//��������
			Show_Str(30,95,200,12,"��������:",12);	//��������
			//atk_8266_wificonf_show(30,160,"������·�������߲���Ϊ:",(u8*)wifista_ssid,(u8*)wifista_encryption,(u8*)wifista_password);
			POINT_COLOR=BLUE;
			//Show_Str(120+30,60,200,12,(u8*)ATK_ESP8266_WORKMODE_TBL[netpro],12); 		//����״̬
			USART2_RX_STA=0;
			
    POINT_COLOR = RED;
    LCD_ShowString(30, 105, 200, 16, 16, "temprature&humidity");
    LCD_ShowString(30, 125, 200, 16, 16, "Bryan ducourt");
		LCD_ShowString(30, 145, 200, 16, 16, "2020.04.23");
    while(AHT10_Init())			//��ʼ��AHT10
    {
        LCD_ShowString(30, 170, 200, 16, 16, "AHT10 Error");
        delay_ms(200);
        LCD_Fill(30, 170, 239, 170 + 16, WHITE);
        delay_ms(200);
    }

    

    while(1)
    {
        if(t % 10 == 0) //ÿ100ms��ȡһ��
        {
					POINT_COLOR = RED;
					LCD_ShowString(30, 170, 200, 16, 16, "AHT10 OK");
					POINT_COLOR = BLUE; //��������Ϊ��ɫ
					LCD_ShowString(30, 190, 200, 16, 16, "Temp:   . C");
					LCD_ShowString(30, 210, 200, 16, 16, "Humi:   . %RH");
            temperature = AHT10_Read_Temperature();
            humidity = AHT10_Read_Humidity();
            if(temperature < 0)
            {
                LCD_ShowChar(30 + 40, 190, '-', 16);	//��ʾ����
                temperature = -temperature;				//תΪ����
            }
            else
                LCD_ShowChar(30 + 40, 190, ' ', 16);	//ȥ������

            LCD_ShowNum(30 + 48, 190, temperature, 2, 16);					//��ʾ�¶�����
            LCD_ShowNum(30 + 72, 190, (u32)(temperature * 10) % 10, 1, 16);	//��ʾ�¶�С��

            LCD_ShowNum(30 + 48, 210, humidity, 2, 16);						//��ʾʪ������
            LCD_ShowNum(30 + 72, 210, (u32)(humidity * 10) % 10, 1, 16);	//��ʾʪ��С��
        }
				
        delay_ms(10);
        t++;
			if (temperature>tempThreshold)
				{
					
					BEEP(1);
					delay_ms(50);
					BEEP(0);
					LED_B(1);
					LED_R_TogglePin;
					t+=5;
				}	
			 else
				LED_B_TogglePin;	//��������ָʾ��
			if ( t % 500 == 0 )
			{
				sprintf((char*)p,"�¶ȣ�%.2f\r\nʪ�ȣ�%.2f\r\n",temperature,humidity);
						atk_8266_quit_trans();
						atk_8266_send_cmd("AT+CIPSEND","OK",20);         //��ʼ͸��           
						
						Show_Str(30+54,80,200,12,p,12);
				sprintf((char*)p,"temperature��%.2f  humidity��%.2f\r\n",temperature,humidity);		
				u2_printf("%s",p);
						timex=100;			
				if(timex)timex--;
				if(timex==1)LCD_Fill(30+54,80,239,112,WHITE);
				t1++;
				delay_ms(10);
				if(USART2_RX_STA&0X8000)		//���յ�һ��������
				{ 
					rlen=USART2_RX_STA&0X7FFF;	//�õ����ν��յ������ݳ���
					USART2_RX_BUF[rlen]=0;		//��ӽ����� 
					printf("%s",USART2_RX_BUF);	//���͵�����   
					sprintf((char*)p,"�յ�%d�ֽ�,��������",rlen);//���յ����ֽ��� 
					LCD_Fill(30+54,95,239,130,WHITE);
					POINT_COLOR=BRED;
					Show_Str(30+54,95,156,12,p,12); 			//��ʾ���յ������ݳ���
					POINT_COLOR=BLUE;
					LCD_Fill(30,110,239,319,WHITE);
					Show_Str(30,110,180,190,USART2_RX_BUF,12);//��ʾ���յ�������  
					USART2_RX_STA=0;
					if(constate!='+')t1=1000;		//״̬Ϊ��δ����,������������״̬
					else t1=0;                   //״̬Ϊ�Ѿ�������,10����ټ��
				}  
				if(t1==1000)//����10����û���յ��κ�����,��������ǲ��ǻ�����.
				{
//					LCD_Fill(30+54,105,239,130,WHITE);
//					LCD_Fill(60,60,120,92,WHITE);
					constate=atk_8266_consta_check();//�õ�����״̬
					if(constate=='+')Show_Str(30+30,60,200,12,"���ӳɹ�",12);  //����״̬
					else Show_Str(30+30,60,200,12,"����ʧ��",12); 	 
					t1=0;
				}
				atk_8266_at_response(1);
			}
			}	
        
			
    
}



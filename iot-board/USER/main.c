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
 *	正点原子 Pandora STM32L475 IoT开发板	实验15
 *	ADC模数转换实验		HAL库版本
 *	技术支持：www.openedv.com
 *	淘宝店铺：http://openedv.taobao.com
 *	关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 *	广州市星翼电子科技有限公司
 *	作者：正点原子 @ALIENTEK
 *	******************************************************************************/

int main(void)
{
	 u8 t=0,tt=0;
		u8 key=0;
		u8 port[4]="8080";
    float temperature, humidity;
    HAL_Init();
    SystemClock_Config();		//初始化系统时钟为80M
    delay_init(80); 			//初始化延时函数    80M系统时钟
    uart_init(115200);			//初始化串口，波特率为115200
	usart2_init(115200);
    LED_Init();					//初始化LED
    LCD_Init();					//初始化LCD
	KEY_Init();					//初始化按键
	W25QXX_Init();				//初始化W25Q256
	usmart_dev.init(80);		//初始化串口调试组件
    my_mem_init(SRAM1);			//初始化内部SRAM1内存池
    my_mem_init(SRAM2);			//初始化内部SRAM2内存池
		BEEP_Init();
    exfuns_init();		            //为fatfs相关变量申请内存
//    f_mount(fs[0], "0:", 1);        //挂载SD卡
    f_mount(fs[1], "1:", 1);        //挂载SPI FLASH.
	
	
	while(font_init()) 		        //检查字库
    {
        LCD_Clear(WHITE);		   	//清屏
        POINT_COLOR = RED;			//设置字体为红色
        Display_ALIENTEK_LOGO(0, 0);
        LCD_ShowString(30, 100, 200, 16, 16, "Pandora STM32L4 IOT");
        while(SD_Init())			//检测SD卡
        {
            LCD_ShowString(30, 120, 200, 16, 16, "SD Card Failed!");
            delay_ms(200);
            LCD_Fill(30, 120, 200 + 30, 120 + 16, WHITE);
            delay_ms(200);
        }
        LCD_ShowString(30, 120, 200, 16, 16, "SD Card OK");
        LCD_ShowString(30, 140, 200, 16, 16, "Font Updating...");
        key = update_font(30, 160, 16, (u8 *)"0:"); //更新字库
        while(key)//更新失败
        {
            LCD_ShowString(30, 160, 200, 16, 16, "Font Update Failed!");
            delay_ms(200);
            LCD_Fill(30, 160, 200 + 20, 160 + 16, WHITE);
            delay_ms(200);
        }
        LCD_ShowString(30, 160, 200, 16, 16, "Font Update Success!            ");
        delay_ms(1500);
        LCD_Clear(WHITE);//清屏
    }
	
	
	
    POINT_COLOR = RED;
	u8 timex=0; 
	u8 ipbuf[16]; 	//IP缓存
	u8 *p;
	u16 t1=999;		//加速第一次获取链接状态
	u8 res=0;
	u16 rlen=0;
	u8 constate=0;	//连接状态
	u8 sramx = 0;
	p=mymalloc(sramx,32);							//申请32字节内存
	atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//设置WIFI STA模式
	atk_8266_send_cmd("AT+RST","OK",20);		//DHCP服务器关闭(仅AP模式有效) 
	delay_ms(1000);         //延时3S等待重启成功
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",wifista_ssid,wifista_password);//设置无线参数:ssid,密码
	while(atk_8266_send_cmd(p,"WIFI GOT IP",300));					//连接目标路由器,并且获得IP

			LCD_Clear(WHITE);
			POINT_COLOR=RED;
			if(atk_8266_ip_set("WIFI-STA 远端IP设置",(u8*)ATK_ESP8266_WORKMODE_TBL[1],(u8*)portnum,ipbuf))//goto PRESTA;	//IP输入
			atk_8266_send_cmd("AT+CIPMUX=0","OK",20);   //0：单连接，1：多连接
			printf("%s",ipbuf);
			sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标TCP服务器
			while(atk_8266_send_cmd(p,"OK",200))
			{
					LCD_Clear(WHITE);
					POINT_COLOR=RED;
					Show_Str_Mid(0,20,"WK_UP:返回重选",16,240);
					Show_Str(30,60,200,12,"ATK-ESP 连接TCP失败",12); //连接失败	 
					key=KEY_Scan(0);
				//	if(key==WKUP_PRES)goto PRESTA;
			}	
			atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //传输模式为：透传			

			LCD_Clear(WHITE);
			POINT_COLOR=RED;
			//Show_Str_Mid(0,10,"ATK-ESP WIFI-STA 测试",16,240);
			//Show_Str(30,30,200,16,"正在配置ATK-ESP模块,请稍等...",12);			
			//LCD_Fill(30,30,239,50+12,WHITE);			//清除之前的显示
			//Show_Str(30,30,200,16,"WK_UP:退出测试  KEY0:发送数据",12);
			//LCD_Fill(30,60,239,80+12,WHITE);
			atk_8266_get_wanip(ipbuf);//服务器模式,获取WAN IP
			sprintf((char*)p,"IP地址:%s 端口:%s",ipbuf,(u8*)portnum);
			Show_Str(30,45,200,12,p,12);				//显示IP地址和端口	
			Show_Str(30,60,200,12,"状态:",12); 		//连接状态
			//Show_Str(120,60,200,12,"模式:",12); 		//连接状态
			Show_Str(30,80,200,12,"发送数据:",12); 	//发送数据
			Show_Str(30,95,200,12,"接收数据:",12);	//接收数据
			//atk_8266_wificonf_show(30,160,"请设置路由器无线参数为:",(u8*)wifista_ssid,(u8*)wifista_encryption,(u8*)wifista_password);
			POINT_COLOR=BLUE;
			//Show_Str(120+30,60,200,12,(u8*)ATK_ESP8266_WORKMODE_TBL[netpro],12); 		//连接状态
			USART2_RX_STA=0;
			
    POINT_COLOR = RED;
    LCD_ShowString(30, 105, 200, 16, 16, "temprature&humidity");
    LCD_ShowString(30, 125, 200, 16, 16, "Bryan ducourt");
		LCD_ShowString(30, 145, 200, 16, 16, "2020.04.23");
    while(AHT10_Init())			//初始化AHT10
    {
        LCD_ShowString(30, 170, 200, 16, 16, "AHT10 Error");
        delay_ms(200);
        LCD_Fill(30, 170, 239, 170 + 16, WHITE);
        delay_ms(200);
    }

    

    while(1)
    {
        if(t % 10 == 0) //每100ms读取一次
        {
					POINT_COLOR = RED;
					LCD_ShowString(30, 170, 200, 16, 16, "AHT10 OK");
					POINT_COLOR = BLUE; //设置字体为蓝色
					LCD_ShowString(30, 190, 200, 16, 16, "Temp:   . C");
					LCD_ShowString(30, 210, 200, 16, 16, "Humi:   . %RH");
            temperature = AHT10_Read_Temperature();
            humidity = AHT10_Read_Humidity();
            if(temperature < 0)
            {
                LCD_ShowChar(30 + 40, 190, '-', 16);	//显示负号
                temperature = -temperature;				//转为正数
            }
            else
                LCD_ShowChar(30 + 40, 190, ' ', 16);	//去掉负号

            LCD_ShowNum(30 + 48, 190, temperature, 2, 16);					//显示温度整数
            LCD_ShowNum(30 + 72, 190, (u32)(temperature * 10) % 10, 1, 16);	//显示温度小数

            LCD_ShowNum(30 + 48, 210, humidity, 2, 16);						//显示湿度整数
            LCD_ShowNum(30 + 72, 210, (u32)(humidity * 10) % 10, 1, 16);	//显示湿度小数
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
				LED_B_TogglePin;	//程序运行指示灯
			if ( t % 500 == 0 )
			{
				sprintf((char*)p,"温度：%.2f\r\n湿度：%.2f\r\n",temperature,humidity);
						atk_8266_quit_trans();
						atk_8266_send_cmd("AT+CIPSEND","OK",20);         //开始透传           
						
						Show_Str(30+54,80,200,12,p,12);
				sprintf((char*)p,"temperature：%.2f  humidity：%.2f\r\n",temperature,humidity);		
				u2_printf("%s",p);
						timex=100;			
				if(timex)timex--;
				if(timex==1)LCD_Fill(30+54,80,239,112,WHITE);
				t1++;
				delay_ms(10);
				if(USART2_RX_STA&0X8000)		//接收到一次数据了
				{ 
					rlen=USART2_RX_STA&0X7FFF;	//得到本次接收到的数据长度
					USART2_RX_BUF[rlen]=0;		//添加结束符 
					printf("%s",USART2_RX_BUF);	//发送到串口   
					sprintf((char*)p,"收到%d字节,内容如下",rlen);//接收到的字节数 
					LCD_Fill(30+54,95,239,130,WHITE);
					POINT_COLOR=BRED;
					Show_Str(30+54,95,156,12,p,12); 			//显示接收到的数据长度
					POINT_COLOR=BLUE;
					LCD_Fill(30,110,239,319,WHITE);
					Show_Str(30,110,180,190,USART2_RX_BUF,12);//显示接收到的数据  
					USART2_RX_STA=0;
					if(constate!='+')t1=1000;		//状态为还未连接,立即更新连接状态
					else t1=0;                   //状态为已经连接了,10秒后再检查
				}  
				if(t1==1000)//连续10秒钟没有收到任何数据,检查连接是不是还存在.
				{
//					LCD_Fill(30+54,105,239,130,WHITE);
//					LCD_Fill(60,60,120,92,WHITE);
					constate=atk_8266_consta_check();//得到连接状态
					if(constate=='+')Show_Str(30+30,60,200,12,"连接成功",12);  //连接状态
					else Show_Str(30+30,60,200,12,"连接失败",12); 	 
					t1=0;
				}
				atk_8266_at_response(1);
			}
			}	
        
			
    
}



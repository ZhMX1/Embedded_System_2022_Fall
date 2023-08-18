/*
*********************************************************************************************************
*
*	模块名称 : 主程序
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 出厂测试程序的目的是测试硬件设备的好坏
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2015-05-04 armfly  正式发布
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"		/* printf函数定向输出到串口，所以必须包含这个文件 */
#include "images.h"		/* 图标文件 */
#include "main.h"
#include "main_menu.h"
#include "touch_test.h"
#include "hard_test.h"
#include "http_server.h"
#include "fm_radio.h"
#include "audio_rec.h"
#include "param.h"
#include "mems_test.h"
//#include "gps_test.h"
#include "usbd_msc_test.h"
#include "ad7606_test.h"
#include "ad7705_test.h"
#include "ads1256_test.h"
#include "camera_test.h"
#include "gprs_test.h"
#include "oled_test.h"
#include "ra8875_flash.h"
#include "mp3_player.h"
#include "form_rs485.h"
#include "form_dac8501.h"
#include "form_dac8562.h"
#include "form_dso.h"
#include "wifi_test.h"
#include "form_step_moto.h"
#include "reserve.h"
#include "form_ad9833.h"
#include "form_can.h"

/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	uint16_t ucStatus;

	/*
		由于ST固件库的启动文件已经执行了CPU系统时钟的初始化，所以不必再次重复配置系统时钟。
		启动文件 startup_stm32f4xx.s 会调用 system_stm32f4xx.c 中的 void SystemInit(void)。
		SystemInit()函数配置了CPU主时钟频率、内部Flash访问速度和可选的外部SRAM FSMC初始化。

		安富莱STM32-V5开发板主晶振是25MHz, 内部PLL倍频到168MHz。如果需要更改主频，可以修改下面的文件：
		\User\bsp_stm32f4xx\system_stm32f4xx.c
		文件开头的几个宏是PLL倍频参数，修改这些宏就可以修改主频，无需更改硬件。
	*/
	bsp_Init();
	
	bsp_InitMsg();	/* 初始化应用层消息 */
	
	LoadParam();	/* 读取应用程序参数, 该函数在param.c */
	
	PrintfLogo();	/* 打印例程Logo到串口1 */		
	
	LCD_InitHard();	/* 初始化显示器硬件(配置GPIO和FSMC,给LCD发送初始化指令) */
	TOUCH_InitHard();

	LCD_ClrScr(CL_BLUE);  		/* 清屏 背景蓝色 */

	LCD_SetBackLight(g_tParam.ucBackLight);		/* 设置背光亮度。 */

	BEEP_KeyTone();		/* 上电鸣叫一声 */
	
	PrintfHardInfo();	/* 上电自检，打印到串口 */

	/* 主程序采用状态机实现程序功能切换 */
	ucStatus = MS_MAIN_MENU;	/* 初始状态 = 图标界面 */
	while (1)
	{
		switch (ucStatus)
		{
			 case MS_CALIBRATION:
			 	TOUCH_Calibration();		/* 校准触摸, 该界面实现在bsp_touch.c */
			 	ucStatus = MS_MAIN_MENU; 	/* 校准完毕后，返回到主界面 */
			 	break;

			 case MS_MAIN_MENU:
			 	ucStatus = MainMenu();		/* 主界面状态 */
			 	break;

//			 case MS_HARDWARE_INFO:			/* 硬件信息 */
//			 	HardInfo();
//			 	ucStatus = MS_MAIN_MENU;
//			 	break;

			 case MS_TEST_TOUCH:			/* 测试触摸屏 */
			 	TestTouch();
			 	ucStatus = MS_MAIN_MENU;
			 	break;

			 case MS_RADIO:					/* Si4730收音机 */
			 	RadioMain();
			 	ucStatus = MS_MAIN_MENU;
			 	break;

//			 case MS_WM8978_REC:			/* WM8978录音机 */
//			 	RecorderDemo();
//			 	ucStatus = MS_MAIN_MENU;
//			 	break;

//			 case MS_ETH_WEB:				/* 网卡测试，DM9000和DM9161 */
//			 	WebServer();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

			 case MS_MEMS:					/* MEMS 传感器测试（I2C） 三轴陀螺仪，磁力计，气压计 */
			 	TestMems();
			 	ucStatus = MS_MAIN_MENU;
			 	break;

//			case MS_GPS:					/* GPS测试 */
//			 	//TestGPS();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_GPRS:					/* GPRS模块测试 */
//			 	TestGPRS();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

			case MS_MP3:
				MP3Player();				/* MP3播放器 */
			 	ucStatus = MS_MAIN_MENU; 	/* 校准完毕后，返回到主界面 */
			 	break;

//			case MS_MASS_STORAGE:			/* SD卡，NAND模拟U盘 */
//			 	TestUsbdMsc();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_CAMERA:					/* 摄像头 */
//				//TestCamera();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_OLED:					/*  OLED显示器 */
//				TestOLED();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_AD7606:					/* AD7606数据采集模块测试 */
//				//TestAD7606();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_AD7705:					/* AD7705模块  */
//				TestAD7705();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_ADS1256:				/* ADS1256模块  */
//				//TestADS1256();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_RS485:					/* RS485通信 */
//				FormMainRS485();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_DAC8501:				/* DAC8501模块 */
//				FormMainDAC8501();
//			 	ucStatus = MS_MAIN_MENU;
//				break;			
//			
//			case MS_DAC8562:				/* DAC8562模块 */
//				FormMainDAC8562();
//			 	ucStatus = MS_MAIN_MENU;
//				break;				

//			case MS_RA8875:					/* RA8875字库 */
//				RA8875FlashMain();
//			 	ucStatus = MS_MAIN_MENU;
//				break;
//			
//			case MS_DSO:					/* 示波器例子 */
//				DsoMain();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_WIFI:					/* WIFI模块 */
//				TestWIFI();
//			 	ucStatus = MS_MAIN_MENU;
//				break;
//			
//			case MS_STEP_MOTO:				/* 步进电机驱动 */
//				FormMainMoto();
//			 	ucStatus = MS_MAIN_MENU;
//				break;		
//			
//			case MS_AD9833:					/* AD9833模块 */
//				FormMain9833();
//			 	ucStatus = MS_MAIN_MENU;
//				break;			

//			case MS_CAN:					/* CAN网络测试 */
//				FormMainCAN();
//			 	ucStatus = MS_MAIN_MENU;
//				break;							
//			
//			case MS_RESERVED:				/* 保留的界面 */
//				ReserveFunc();
//				ucStatus = MS_MAIN_MENU;
//				break;
			
			default:
				ucStatus = MS_MAIN_MENU;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST固件库版本，宏定义在 stm32f4xx.h 文件 */
	printf("* 固件库版本 : %d.%d.%d\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
			__STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);

	/* 打印 CMSIS 版本. 宏定义在 core_cm4.h 文件 */
	printf("* CMSIS版本  : %X.%02X\r\n", __CM3_CMSIS_VERSION_MAIN, __CM3_CMSIS_VERSION_SUB);

	printf("* \n\r");	/* 打印一行空格 */
	printf("* QQ    : 1295744630 \r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* Copyright www.armfly.com 安富莱电子\r\n");
	printf("*************************************************************\n\r");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

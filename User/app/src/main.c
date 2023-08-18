/*
*********************************************************************************************************
*
*	ģ������ : ������
*	�ļ����� : main.c
*	��    �� : V1.0
*	˵    �� : �������Գ����Ŀ���ǲ���Ӳ���豸�ĺû�
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2015-05-04 armfly  ��ʽ����
*
*	Copyright (C), 2015-2016, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"		/* printf����������������ڣ����Ա����������ļ� */
#include "images.h"		/* ͼ���ļ� */
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

/* �������ļ��ڵ��õĺ������� */
static void PrintfLogo(void);

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{
	uint16_t ucStatus;

	/*
		����ST�̼���������ļ��Ѿ�ִ����CPUϵͳʱ�ӵĳ�ʼ�������Բ����ٴ��ظ�����ϵͳʱ�ӡ�
		�����ļ� startup_stm32f4xx.s ����� system_stm32f4xx.c �е� void SystemInit(void)��
		SystemInit()����������CPU��ʱ��Ƶ�ʡ��ڲ�Flash�����ٶȺͿ�ѡ���ⲿSRAM FSMC��ʼ����

		������STM32-V5��������������25MHz, �ڲ�PLL��Ƶ��168MHz�������Ҫ������Ƶ�������޸�������ļ���
		\User\bsp_stm32f4xx\system_stm32f4xx.c
		�ļ���ͷ�ļ�������PLL��Ƶ�������޸���Щ��Ϳ����޸���Ƶ���������Ӳ����
	*/
	bsp_Init();
	
	bsp_InitMsg();	/* ��ʼ��Ӧ�ò���Ϣ */
	
	LoadParam();	/* ��ȡӦ�ó������, �ú�����param.c */
	
	PrintfLogo();	/* ��ӡ����Logo������1 */		
	
	LCD_InitHard();	/* ��ʼ����ʾ��Ӳ��(����GPIO��FSMC,��LCD���ͳ�ʼ��ָ��) */
	TOUCH_InitHard();

	LCD_ClrScr(CL_BLUE);  		/* ���� ������ɫ */

	LCD_SetBackLight(g_tParam.ucBackLight);		/* ���ñ������ȡ� */

	BEEP_KeyTone();		/* �ϵ�����һ�� */
	
	PrintfHardInfo();	/* �ϵ��Լ죬��ӡ������ */

	/* ���������״̬��ʵ�ֳ������л� */
	ucStatus = MS_MAIN_MENU;	/* ��ʼ״̬ = ͼ����� */
	while (1)
	{
		switch (ucStatus)
		{
			 case MS_CALIBRATION:
			 	TOUCH_Calibration();		/* У׼����, �ý���ʵ����bsp_touch.c */
			 	ucStatus = MS_MAIN_MENU; 	/* У׼��Ϻ󣬷��ص������� */
			 	break;

			 case MS_MAIN_MENU:
			 	ucStatus = MainMenu();		/* ������״̬ */
			 	break;

//			 case MS_HARDWARE_INFO:			/* Ӳ����Ϣ */
//			 	HardInfo();
//			 	ucStatus = MS_MAIN_MENU;
//			 	break;

			 case MS_TEST_TOUCH:			/* ���Դ����� */
			 	TestTouch();
			 	ucStatus = MS_MAIN_MENU;
			 	break;

			 case MS_RADIO:					/* Si4730������ */
			 	RadioMain();
			 	ucStatus = MS_MAIN_MENU;
			 	break;

//			 case MS_WM8978_REC:			/* WM8978¼���� */
//			 	RecorderDemo();
//			 	ucStatus = MS_MAIN_MENU;
//			 	break;

//			 case MS_ETH_WEB:				/* �������ԣ�DM9000��DM9161 */
//			 	WebServer();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

			 case MS_MEMS:					/* MEMS ���������ԣ�I2C�� ���������ǣ������ƣ���ѹ�� */
			 	TestMems();
			 	ucStatus = MS_MAIN_MENU;
			 	break;

//			case MS_GPS:					/* GPS���� */
//			 	//TestGPS();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_GPRS:					/* GPRSģ����� */
//			 	TestGPRS();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

			case MS_MP3:
				MP3Player();				/* MP3������ */
			 	ucStatus = MS_MAIN_MENU; 	/* У׼��Ϻ󣬷��ص������� */
			 	break;

//			case MS_MASS_STORAGE:			/* SD����NANDģ��U�� */
//			 	TestUsbdMsc();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_CAMERA:					/* ����ͷ */
//				//TestCamera();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_OLED:					/*  OLED��ʾ�� */
//				TestOLED();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_AD7606:					/* AD7606���ݲɼ�ģ����� */
//				//TestAD7606();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_AD7705:					/* AD7705ģ��  */
//				TestAD7705();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_ADS1256:				/* ADS1256ģ��  */
//				//TestADS1256();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_RS485:					/* RS485ͨ�� */
//				FormMainRS485();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_DAC8501:				/* DAC8501ģ�� */
//				FormMainDAC8501();
//			 	ucStatus = MS_MAIN_MENU;
//				break;			
//			
//			case MS_DAC8562:				/* DAC8562ģ�� */
//				FormMainDAC8562();
//			 	ucStatus = MS_MAIN_MENU;
//				break;				

//			case MS_RA8875:					/* RA8875�ֿ� */
//				RA8875FlashMain();
//			 	ucStatus = MS_MAIN_MENU;
//				break;
//			
//			case MS_DSO:					/* ʾ�������� */
//				DsoMain();
//			 	ucStatus = MS_MAIN_MENU;
//				break;

//			case MS_WIFI:					/* WIFIģ�� */
//				TestWIFI();
//			 	ucStatus = MS_MAIN_MENU;
//				break;
//			
//			case MS_STEP_MOTO:				/* ����������� */
//				FormMainMoto();
//			 	ucStatus = MS_MAIN_MENU;
//				break;		
//			
//			case MS_AD9833:					/* AD9833ģ�� */
//				FormMain9833();
//			 	ucStatus = MS_MAIN_MENU;
//				break;			

//			case MS_CAN:					/* CAN������� */
//				FormMainCAN();
//			 	ucStatus = MS_MAIN_MENU;
//				break;							
//			
//			case MS_RESERVED:				/* �����Ľ��� */
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
*	�� �� ��: PrintfLogo
*	����˵��: ��ӡ�������ƺ����̷�������, ���ϴ����ߺ󣬴�PC���ĳ����ն�������Թ۲���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	printf("* ��������   : %s\r\n", EXAMPLE_NAME);	/* ��ӡ�������� */
	printf("* ���̰汾   : %s\r\n", DEMO_VER);		/* ��ӡ���̰汾 */
	printf("* ��������   : %s\r\n", EXAMPLE_DATE);	/* ��ӡ�������� */

	/* ��ӡST�̼���汾���궨���� stm32f4xx.h �ļ� */
	printf("* �̼���汾 : %d.%d.%d\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
			__STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);

	/* ��ӡ CMSIS �汾. �궨���� core_cm4.h �ļ� */
	printf("* CMSIS�汾  : %X.%02X\r\n", __CM3_CMSIS_VERSION_MAIN, __CM3_CMSIS_VERSION_SUB);

	printf("* \n\r");	/* ��ӡһ�пո� */
	printf("* QQ    : 1295744630 \r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* Copyright www.armfly.com ����������\r\n");
	printf("*************************************************************\n\r");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

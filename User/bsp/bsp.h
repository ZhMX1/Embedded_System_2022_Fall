/*
*********************************************************************************************************
*
*	ģ������ : BSPģ��
*	�ļ����� : bsp.h
*	˵    �� : ���ǵײ�����ģ�����е�h�ļ��Ļ����ļ��� Ӧ�ó���ֻ�� #include bsp.h ����.
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_H_
#define __BSP_H

/*
	stm32f10x.h :STM32F10xϵ��CPU�ı�׼����̼��⡣����̼������˺ܶ��ͺŵ�CPU��
		ʹ�����������ڹ��������Ԥ����ĺ�:
			USE_STDPERIPH_DRIVER  ��ʾ���ñ�׼�����
			STM32F10X_HD          ��ʾCPU��10x�������汾(HD = High Density)
	
	stdlib.h : standard library��׼��ͷ�ļ�,������C��C++���Ե���õ����Ͷ����ϵͳ���������� malloc()��free()
	
	stdio.h : standard input & output ��׼������������⡣���ú����� printf()��getc()��putc()
	
	string.h : �����ַ�����ĺ��������ͷ�ļ��⣬���ú�����strlen��strcmp��strcpy
	
	" " �� < > ������:
	" " һ���ʾԴ�����Դ���h�ļ�������ʱ�������������ڹ�������ָ��������·���б���Ѱ��ͷ�ļ�
	< > һ���ʾ����ϵͳ�Դ���h�ļ�������ʱ��������������IDE�Դ��Ŀ��ļ���Ѱ��ͷ�ļ���
*/
#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ���� BSP �汾�� */
#define __STM32F1_BSP_VERSION		"1.1"

/* ����ȫ���жϵĺ� */
#define ENABLE_INT()	__set_PRIMASK(0)	/* ʹ��ȫ���ж� */
#define DISABLE_INT()	__set_PRIMASK(1)	/* ��ֹȫ���ж� */

/* ���������ڵ��Խ׶��Ŵ���Ҫ�����ڴ����еĵ���������ʹ�� BSP_Printf ����� printf */
#if 1	/* 1��ʾʹ�ܵ�����䣬 0 ��ʾ��������룬����Ӱ������� */
	#define BSP_Printf		printf
#else
	#define BSP_Printf(...)
#endif

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

/*
	EXTI9_5_IRQHandler ���жϷ�������ɢ�ڼ��������� bsp�ļ��С�
	��Ҫ���ϵ� stm32f10x_it.c �С�

	���������б�ʾEXTI9_5_IRQHandler��ں������зŵ� stm32f10x_it.c�ļ�
*/
#define EXTI9_5_ISR_MOVE_OUT

#define DEBUG_GPS_TO_COM1		/* ��ӡGPS���ݵ�����1 */

/* ͨ��ȡ��ע�ͻ������ע�͵ķ�ʽ�����Ƿ�����ײ�����ģ�� */
#include "bsp_user_lib.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "bsp_timer.h"
#include "bsp_msg.h"
#include "bsp_uart_fifo.h"
#include "bsp_sdio_sd.h"
#include "bsp_cpu_rtc.h"
#include "bsp_cpu_flash.h"
#include "bsp_tim_pwm.h"
#include "bsp_beep.h"

#include "bsp_i2c_gpio.h"
#include "bsp_eeprom_24xx.h"
#include "bsp_si4730.h"
#include "bsp_hmc5883l.h"
#include "bsp_mpu6050.h"
#include "bsp_bh1750.h"
#include "bsp_bmp180.h"
#include "bsp_wm8978.h"
#include "bsp_gt811.h"

#include "bsp_fsmc_sram.h"
#include "bsp_nand_flash.h"
#include "bsp_nor_flash.h"

#include "LCD_RA8875.h"
#include "LCD_SPFD5420.h"
#include "LCD_ILI9488.h"
#include "bsp_ra8875_port.h"
#include "bsp_tft_lcd.h"
#include "bsp_touch.h"
#include "bsp_ts_ft5x06.h"

#include "bsp_oled.h"
#include "bsp_sim800.h"
#include "bsp_ra8875_flash.h"

#include "bsp_spi_bus.h"
#include "bsp_spi_flash.h"
#include "bsp_tm7705.h"
#include "bsp_vs1053b.h"
#include "bsp_tsc2046.h"

#include "bsp_ds18b20.h"
#include "bsp_dac8501.h"
#include "bsp_dht11.h"

#include "bsp_ir_decode.h"
#include "bsp_ps2.h"

#include "bsp_modbus.h"
#include "bsp_rs485_led.h"

#include "bsp_dac8501.h"
#include "bsp_dac8562.h"

#include "bsp_esp8266.h"
#include "bsp_step_moto.h"
#include "bsp_ad9833.h"

/* �ṩ������C�ļ����õĺ��� */
void bsp_Init(void);
void bsp_Idle(void);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

/*
*********************************************************************************************************
*
*	模块名称 : BSP模块
*	文件名称 : bsp.h
*	说    明 : 这是底层驱动模块所有的h文件的汇总文件。 应用程序只需 #include bsp.h 即可.
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_H_
#define __BSP_H

/*
	stm32f10x.h :STM32F10x系列CPU的标准外设固件库。这个固件包含了很多型号的CPU，
		使用这个库必须在工程中添加预编译的宏:
			USE_STDPERIPH_DRIVER  表示启用标准外设库
			STM32F10X_HD          表示CPU是10x大容量版本(HD = High Density)
	
	stdlib.h : standard library标准库头文件,包含了C、C++语言的最常用的类型定义和系统函数，比如 malloc()、free()
	
	stdio.h : standard input & output 标准输入输出函数库。常用函数有 printf()、getc()、putc()
	
	string.h : 关于字符数组的函数定义的头文件库，常用函数有strlen、strcmp、strcpy
	
	" " 和 < > 的区别:
	" " 一般表示源代码自带的h文件。编译时，编译器优先在工程配置指定的搜索路径列表中寻找头文件
	< > 一般表示编译系统自带的h文件。编译时，编译器优先在IDE自带的库文件夹寻找头文件。
*/
#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 定义 BSP 版本号 */
#define __STM32F1_BSP_VERSION		"1.1"

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */

/* 这个宏仅用于调试阶段排错。需要保留在代码中的调试语句可以使用 BSP_Printf 宏替代 printf */
#if 1	/* 1表示使能调试语句， 0 表示不参与编译，不会影响编译结果 */
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
	EXTI9_5_IRQHandler 的中断服务程序分散在几个独立的 bsp文件中。
	需要整合到 stm32f10x_it.c 中。

	定义下面行表示EXTI9_5_IRQHandler入口函数集中放到 stm32f10x_it.c文件
*/
#define EXTI9_5_ISR_MOVE_OUT

#define DEBUG_GPS_TO_COM1		/* 打印GPS数据到串口1 */

/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */
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

/* 提供给其他C文件调用的函数 */
void bsp_Init(void);
void bsp_Idle(void);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

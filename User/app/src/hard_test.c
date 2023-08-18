/*
*********************************************************************************************************
*
*	模块名称 : 硬件测试界面。
*	文件名称 : hard_test.c
*	版    本 : V1.0
*	说    明 : 显示硬件信息。自动测试一些芯片。
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-01-01 armfly  正式发布
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"			/* printf函数定向输出到串口，所以必须包含这个文件 */
#include "hard_test.h"

/* 返回按钮的坐标(屏幕右下角) */
#define BUTTON_RET_H	32
#define BUTTON_RET_W	60
#define	BUTTON_RET_X	(g_LcdWidth - BUTTON_RET_W - 4)
#define	BUTTON_RET_Y	(g_LcdHeight - BUTTON_RET_H - 4)


/*
*********************************************************************************************************
*	函 数 名: HardInfo
*	功能说明: 自动检测硬件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void HardInfo(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	uint8_t fRefresh;		/* 刷屏请求标志,1表示需要刷新 */
	FONT_T tFont, tFontBtn;	/* 定义一个字体结构体变量，用于设置字体参数 */
	char buf[128];
	uint16_t x, y;
	uint16_t usLineCap = 18;
	int16_t tpX, tpY;
	BUTTON_T tBtn;
	uint8_t fQuit;

	LCD_ClrScr(CL_BLUE);  	/* 清屏，背景蓝色 */

	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
		tFont.FrontColor = CL_WHITE;	/* 字体颜色 */
		tFont.BackColor = CL_BLUE;	/* 文字背景颜色 */
		tFont.Space = 0;				/* 文字间距，单位 = 像素 */

		/* 按钮字体 */
		tFontBtn.FontCode = FC_ST_16;
		tFontBtn.BackColor = CL_MASK;	/* 透明色 */
		tFontBtn.FrontColor = CL_BLACK;
		tFontBtn.Space = 0;
	}
	
	

	x = 5;
	y = 3;
	LCD_DispStr(x, y, "安富莱STM32-V4开发板硬件信息", &tFont);			/* 在(8,3)坐标处显示一串汉字 */
	y += usLineCap;

	/* 检测CPU ID */
	{
		/* 参考手册：
			32.6.1 MCU device ID code
			33.1 Unique device ID register (96 bits)
		*/
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;

		CPU_Sn0 = *(__IO uint32_t*)(0x1FFFF7E8);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FFFF7EC);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FFFF7F0);

		printf("\r\nCPU : STM32F103ZET6, LQFP144\r\n");
		
		LCD_DispStr(x, y, "CPU : STM32F103ZET6, LQFP144", &tFont);			
		y += usLineCap;				
		
		sprintf(buf, " UID = %08X %08X %08X"
			, CPU_Sn2, CPU_Sn1, CPU_Sn0);
		printf("%s\r\n", buf);
		LCD_DispStr(x, y, buf, &tFont);
		y += usLineCap;		
	}
	
	/* 显示TFT控制器型号和屏幕分辨率 */
	{
		strcpy(buf, "TFT Driver : ");
		LCD_GetChipDescribe(&buf[strlen(buf)]);	/* 读取TFT驱动芯片型号 */
		sprintf(&buf[strlen(buf)], "   %d x %d", LCD_GetWidth(), LCD_GetHeight());
		LCD_DispStr(x, y, buf, &tFont);
		y += usLineCap;
	}

	/* 测试SRAM */
	{
		if (bsp_TestExtSRAM() == 0)
		{
			sprintf(buf, "SRAM Model : EM681FV16BU, Test OK");
			printf("%s\r\n", buf);

			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "SRAM Model: EM681FV16BU, Test Err");
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
	}

	/* 测试NOR Flash */
	{
		uint32_t id;

		id = NOR_ReadID();

		if (id == S29GL128P)
		{
			sprintf(buf, "NOR Flash ID = 0x%08X, Model = S29GL128P, OK", id);
			printf("%s\r\n", buf);

			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "NOR Flash ID = 0x%08X, Model = xxxx, Err", id);
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
	}

	/* 测试NADN Flash */
	{
		uint32_t id;

		NAND_Init();	/* 初始化NAND */
		
		id = NAND_ReadID();

		sprintf(buf, "NAND Flash ID = 0x%04X, Type = ", id);
		if (id == HY27UF081G2A)
		{
			sprintf(buf, "NAND Flash ID = 0x%04X, Model = HY27UF081G2A, OK", id);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else if (id == H27U1G8F2BTR)
		{
			sprintf(buf, "NAND Flash ID = 0x%04X, Model = H27U1G8F2BTR, OK", id);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}		
		else if (id == H27U4G8F2DTR)
		{
			sprintf(buf, "NAND Flash ID = 0x%04X, Model = H27U4G8F2DTR, OK", id);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}		
		else
		{
			sprintf(buf, "NAND Flash ID = 0x%04X, Model = Unknow, Err", id);
			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
		//NAND_DispBadBlockInfo();	/* 向串口1打印NAND Flash坏块信息 (此函数开头初始FSMC) */
	}

	/* 测试I2C设备 */
	{
		if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
		{
			sprintf(buf, "AT24C128 Ok (0x%02X)", EE_DEV_ADDR);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "AT24C128 Err (0x%02X)", EE_DEV_ADDR);
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;

		if (i2c_CheckDevice(I2C_ADDR_SI4730_W) == 0)
		{
			sprintf(buf, "Si4704 Ok (0x%02X)", I2C_ADDR_SI4730_W);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "Si4704 Err (0x%02X)", I2C_ADDR_SI4730_W);
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;

#if 0
		if (i2c_CheckDevice(HMC5883L_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "HMC5883L Ok (0x%02X)", HMC5883L_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "HMC5883L Err");
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
#endif		

		if (i2c_CheckDevice(MPU6050_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "MPU6050 Ok (0x%02X)", MPU6050_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "MPU6050 Err (0x%02X)", MPU6050_SLAVE_ADDRESS);
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;

	#if 1
		if (i2c_CheckDevice(BMP180_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "BMP180 Ok (0x%02X)", BMP180_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "BMP180 Err (0x%02X)", BMP180_SLAVE_ADDRESS);
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
	#endif

		if (i2c_CheckDevice(WM8978_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "WM8978 Ok (0x%02X)", WM8978_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "WM8978 Err (0x%02X)", WM8978_SLAVE_ADDRESS);
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;

		if (i2c_CheckDevice(BH1750_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "BH1750 Ok (0x%02X)", BH1750_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "BH1750 Err (0x%02X)", BH1750_SLAVE_ADDRESS);
			printf("%s\r\n", buf);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
	}

	/* 测试SPI Flash */
	{
		bsp_InitSFlash();	/* 初始化SPI硬件 */

		sf_ReadInfo();
		
		/* 检测串行Flash OK */
		sprintf(buf, "SPI Flash ID = %08X, Model = %s",g_tSF.ChipID , g_tSF.ChipName);
		printf("%s\r\n", buf);

		if (g_tSF.ChipID == W25Q128_ID)
		{
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
	}

	fRefresh = 1;	/* 1表示需要刷新LCD */
	bsp_StartAutoTimer(0, 1000);
	fQuit = 0;
	while (fQuit == 0)
	{
		bsp_Idle();

		/* 显示时钟 */
		if (bsp_CheckTimer(0))
		{
			uint16_t x, y;
			
			tFont.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
			tFont.FrontColor = CL_WHITE;	/* 字体颜色 */
			tFont.BackColor = CL_BLUE;	/* 文字背景颜色 */
			tFont.Space = 0;				/* 文字间距，单位 = 像素 */
			
			RTC_ReadClock();	/* 读时钟，结果在 g_tRTC */
			
			x = 5;
			y = LCD_GetHeight() - 20;
			
			sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d",
				g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day, g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
			LCD_DispStr(x, y, buf, &tFont);
		}
		
		if (fRefresh)
		{
			fRefresh = 0;

			/* 显示按钮 */
			{
				tBtn.Font = &tFontBtn;

				tBtn.Left = BUTTON_RET_X;
				tBtn.Top = BUTTON_RET_Y;
				tBtn.Height = BUTTON_RET_H;
				tBtn.Width = BUTTON_RET_W;
				tBtn.Focus = 0;	/* 失去焦点 */
				tBtn.pCaption = "返回";
				LCD_DrawButton(&tBtn);
			}
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
					if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 1;	/* 焦点 */
						tBtn.pCaption = "返回";
						LCD_DrawButton(&tBtn);
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 1;	/* 焦点 */
						tBtn.pCaption = "返回";
						LCD_DrawButton(&tBtn);

						fQuit = 1;		/* 返回 */
					}
					else	/* 按钮失去焦点 */
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 0;	/* 焦点 */
						tBtn.pCaption = "返回";
						LCD_DrawButton(&tBtn);
					}
					break;
			}
		}

		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case  JOY_DOWN_OK:		/* 摇杆OK键 */
					fQuit = 1;
					break;

				default:
					break;
			}
		}
	}
	
	bsp_StopTimer(0);
}


/*
*********************************************************************************************************
*	函 数 名: PrintfHardInfo
*	功能说明: 自动检测硬件. printf到串口。不显示。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
extern uint32_t dm9k_ReadID(void);
void PrintfHardInfo(void)
{
	char buf[128];

	/* 检测CPU ID */
	{
		/* 参考手册：
			32.6.1 MCU device ID code
			33.1 Unique device ID register (96 bits)
		*/
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;

		CPU_Sn0 = *(__IO uint32_t*)(0x1FFFF7E8);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FFFF7EC);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FFFF7F0);

		printf("\r\nCPU : STM32F103ZET6, LQFP144\r\n");
		sprintf(buf, " UID = %08X %08X %08X"
			, CPU_Sn2, CPU_Sn1, CPU_Sn0);
		printf("%s\r\n", buf);
		
	}
	
	/* 显示TFT控制器型号和屏幕分辨率 */
	{
		strcpy(buf, "TFT Driver : ");
		LCD_GetChipDescribe(&buf[strlen(buf)]);	/* 读取TFT驱动芯片型号 */
		sprintf(&buf[strlen(buf)], "   %d x %d", LCD_GetWidth(), LCD_GetHeight());
		printf("%s\r\n", buf);
	}

	/* 测试SRAM */
	#if 1
	{
		if (bsp_TestExtSRAM() == 0)
		{
			sprintf(buf, "SRAM Model : EM681FV16BU, Test OK");
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "SRAM Model: EM681FV16BU, Test Err");
			printf("%s\r\n", buf);
		}
	}


	/* 测试NOR Flash */
	{
		uint32_t id;

		id = NOR_ReadID();

		if (id == S29GL128P)
		{
			sprintf(buf, "NOR Flash ID = 0x%08X, Model = S29GL128P, OK", id);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "NOR Flash ID = 0x%08X, Model = xxxx, Err", id);
			printf("%s\r\n", buf);
		}
	}

	/* 测试NADN Flash */
	{
		uint32_t id;

		NAND_Init();	/* 初始化NAND */
		
		id = NAND_ReadID();

		sprintf(buf, "NAND Flash ID = 0x%04X, Type = ", id);
		if (id == HY27UF081G2A)
		{
			sprintf(buf, "NAND Flash ID = 0x%04X, Model = HY27UF081G2A, OK", id);
			printf("%s\r\n", buf);
		}
		else if (id == H27U1G8F2BTR)
		{
			sprintf(buf, "NAND Flash ID = 0x%04X, Model = H27U1G8F2BTR, OK", id);
			printf("%s\r\n", buf);
		}		
		else if (id == H27U4G8F2DTR)
		{
			sprintf(buf, "NAND Flash ID = 0x%04X, Model = H27U4G8F2DTR, OK", id);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "NAND Flash ID = 0x%04X, Model = Unknow, Err", id);
			printf("%s\r\n", buf);
		}
	}
	#endif
	/* 测试I2C设备 */
	{
		if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
		{
			sprintf(buf, "AT24C128 Ok (0x%02X)", EE_DEV_ADDR);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "AT24C128 Err (0x%02X)", EE_DEV_ADDR);
			printf("%s\r\n", buf);
		}

		if (i2c_CheckDevice(I2C_ADDR_SI4730_W) == 0)
		{
			sprintf(buf, "Si4704 Ok (0x%02X)", I2C_ADDR_SI4730_W);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "Si4704 Err (0x%02X)", I2C_ADDR_SI4730_W);
			printf("%s\r\n", buf);
		}
#if 0
		if (i2c_CheckDevice(HMC5883L_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "HMC5883L Ok (0x%02X)", HMC5883L_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "HMC5883L Err (0x%02X)", HMC5883L_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}
#endif
		if (i2c_CheckDevice(MPU6050_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "MPU6050 Ok (0x%02X)", MPU6050_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "MPU6050 Err (0x%02X)", MPU6050_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}

		if (i2c_CheckDevice(BMP180_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "BMP180 Ok (0x%02X)", BMP180_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "BMP180 Err (0x%02X)", BMP180_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}

		if (i2c_CheckDevice(WM8978_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "WM8978 Ok (0x%02X)", WM8978_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "WM8978 Err (0x%02X)", WM8978_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}

		if (i2c_CheckDevice(BH1750_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "BH1750 Ok (0x%02X)", BH1750_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}
		else
		{
			sprintf(buf, "BH1750 Err (0x%02X)", BH1750_SLAVE_ADDRESS);
			printf("%s\r\n", buf);
		}
	}

	/* 测试SPI Flash */
	{
		//bsp_InitSFlash();	/* 初始化SPI硬件 */

		/* 检测串行Flash OK */
		sprintf(buf, "SPI Flash ID= = %08X, Model = %s",g_tSF.ChipID , g_tSF.ChipName);
		printf("%s\r\n", buf);
	}
	
	/* 检测DM9000 */
	{
		uint32_t id;
		
		id = dm9k_ReadID();
		
		if (id == 0x0A469000)
		{
			printf("DM9000AEP OK,  ID = %8X\r\n", id);
		}
		else
		{
			printf("DM9000AEP Error, ID = %8X\r\n", id);
		}
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

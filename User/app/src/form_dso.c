/*
*********************************************************************************************************
*
*	模块名称 : 双通道示波器程序
*	文件名称 : form_dso.c
*	版    本 : V1.0
*	说    明 : 使用STM32内部ADC测量波形。CH1 = PC0， CH2 = PC1
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2015-06-23  armfly  正式发布
*		V1.1    2015-08-07  armfly  使用堆栈保存大尺寸的数据缓冲区，解决全局变量。
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_dso.h"
#include "WaveMaker.h"

#define DSO_VER	"V0.6"		  /* 当前版本 */

/* 主程序状态字 */
enum
{
	S_HELP		= 0,	/* 帮助页面 */
	S_RUN		= 1,	/* 采集状态 */

	S_EXIT		= 3
};

/* 时间分度表， g_DSO->TimeBaseId作为数组索引  */
const uint32_t g_TimeTable[] =
{
	20, 	50, 	100, 
	200,	500,	1000,
	2000,	5000,	10000,
	20000,	50000,	100000,
	200000,	500000,	1000000		
};

/* 衰减倍数表  


计算公式：
  iTemp = g_DSO->Ch1VOffset + (int16_t)((2024 - g_DSO->Ch1Buf[i + 1]) * 10) / g_DSO->Ch1Attenuation;

	g_DSO->Ch1Attenuation 是ADC值和像素之间的倍率的10倍。

	1V 档位时:  ADC = 4096 / 5.0 = 819； 像素 = 25
	g_DSO->Ch1Attenuation = 819 / 25 = 32.76

*/
#define ATT_COUNT	6
#define Y_RATE  327
const uint32_t g_AttTable[ATT_COUNT][2] =
{
	/* {除数*0.1, 每大格电压}  1:1 */
	{Y_RATE * 5,	5000},
	{Y_RATE * 2,	2000},
	{Y_RATE,	1000},   /*　缺省值 1大格25像素，对应1V.　*/
	{Y_RATE / 2,	500},
	{Y_RATE / 5,	200},
	{Y_RATE / 10,	100},
};

static void DsoHelp(uint8_t *pMainStatus);
static void DsoRun(uint8_t *pMainStatus);

/*
	安富莱STM32-V4 口线分配：

	示波器通道1
		PC0 = ADC1_IN10

	示波器通道2
		PC1 = ADC2_IN12
*/

/* 按钮 */
/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H	32
#define BTN_RET_W	80
#define	BTN_RET_X	(g_LcdWidth - BTN_RET_W - 8)
#define	BTN_RET_Y	(g_LcdHeight - BTN_RET_H - 4)
#define	BTN_RET_T	"返回"

DSO_T *g_DSO;	/* 全局变量，是一个结构体 */

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBtn;		/* 按钮的字体 */
	
	BUTTON_T BtnRet;
}FormDSO_T;

FormDSO_T *FormDSO;

/*
*********************************************************************************************************
*	函 数 名: DsoMain
*	功能说明: 示波器程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DsoMain(void)
{
	uint8_t MainStatus = S_RUN;		/* 程序执行状态 */
	DSO_T tDSO;			/* 很大的一个变量，存放在堆栈。节约全局变量空间 */
	
	memset(&tDSO, 0, sizeof(tDSO));
	g_DSO = &tDSO;

	InitWaveMaker();	/* 使用STM32内部DAC输出正弦波 */
	
	bsp_InitAD9833();				/* 使用外部AD9833模块输出三角波 */

	
	/* 进入主程序循环体 */
	while (1)
	{
		switch (MainStatus)
		{
			case S_HELP:
				DsoHelp(&MainStatus);		/* 显示帮助 */
				break;

			case S_RUN:
				DsoRun(&MainStatus);		/* 全速采集，实时显示 */
				break;

			case S_EXIT:
				AD9833_SelectWave(NONE_WAVE);	/* 关闭输出 */
				return;
			
			default:
				break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: InitDSO
*	功能说明: 对示波器通道1进行初始化配置。主要完成GPIO的配置、ADC的配置、定时器的配置以及DMA的配置。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitDSO(void)
{
	/* 配置GPIO.  */
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* 打开GPIO_C 和 AFIO 的时钟 */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC	| RCC_APB2Periph_AFIO, ENABLE);

		/* 配置PC0为模拟输入模式 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
	}


	/* 配置DMA1_1 用于CH1 */
	#if 1
	{
		DMA_InitTypeDef DMA_InitStructure;

		/* Enable DMA1 clock */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		DMA_DeInit(DMA1_Channel1);		/* 复位DMA1寄存器到缺省状态 */
		DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	/* 选择ADC1的数据寄存器作为源 */
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_DSO->Ch1Buf;	/* 目标地址 */
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* 设置DMA传输方向，外设(ADC)作为源 */
		DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* 设置缓冲区大小 */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* 外设地址不自增 */
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* 存储器地址需要自增 */
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* 选择外设传输单位：16bit */
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* 选择内存传输单位：16bit */
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* 无需循环模式   */
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;	/* 选择DMA优先级 */
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA传输类型，不是内存到内存 */
		DMA_Init(DMA1_Channel1, &DMA_InitStructure);

		/* Enable DMA1 channel1 */
		DMA_Cmd(DMA1_Channel1, ENABLE);
	}
	#endif

	/* 配置DMA2_5 用于CH2 */
	#if 1
	{
		DMA_InitTypeDef DMA_InitStructure;

		/* Enable DMA1 clock */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

		DMA_DeInit(DMA2_Channel5);		/* 复位DMA2寄存器到缺省状态 */
		DMA_InitStructure.DMA_PeripheralBaseAddr = ADC3_DR_Address;	/* 选择ADC3的数据寄存器作为源 */
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_DSO->Ch2Buf;	/* 目标地址 */
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* 设置DMA传输方向，外设(ADC)作为源 */
		DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* 设置缓冲区大小 */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* 外设地址不自增 */
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* 存储器地址需要自增 */
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* 选择外设传输单位：16bit */
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* 选择内存传输单位：16bit */
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* 无需循环模式   */
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;	/* 选择DMA优先级 */
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA传输类型，不是内存到内存 */
		DMA_Init(DMA2_Channel5, &DMA_InitStructure);

		/* Enable DMA2 channel5 */
		DMA_Cmd(DMA2_Channel5, ENABLE);
	}
	#endif

	/* 配置ADC1  */
	
	 /* ADCCLK = PCLK2/2 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);     //ADCCLK = PCLK2/4 = 18MHz

	#if 1
	{
		ADC_InitTypeDef	ADC_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		/* 连续转换静止 */
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;	/* 选择TIM1的CC3做触发 */
		//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;	/* 选择TIM2的CC2做触发 */

		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	/* 数据右对齐,高位为0 */
		ADC_InitStructure.ADC_NbrOfChannel = 1;	/* 1个通道 */
		ADC_Init(ADC1, &ADC_InitStructure);

		/* ADC1 regular channels configuration */
		ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_1Cycles5);

		/* Enable ADC1 external trigger conversion */
		ADC_ExternalTrigConvCmd(ADC1, ENABLE);

		/* 使能 ADC1 */
		ADC_Cmd(ADC1, ENABLE);

		/* Enable ADC1 DMA */
		ADC_DMACmd(ADC1, ENABLE);

		/* 使能 ADC1 复位校准寄存器 */
		ADC_ResetCalibration(ADC1);
		/* 检测复位校准寄存器 */
		while(ADC_GetResetCalibrationStatus(ADC1));

		/* 开始 ADC1 校准 */
		ADC_StartCalibration(ADC1);
		/* 等待校准结束 */
		while(ADC_GetCalibrationStatus(ADC1));
	}
	#endif

	/* 配置ADC3  */
	#if 1
	{
		ADC_InitTypeDef	ADC_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		/* 连续转换静止 */
		//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;	/* 选择TIM1的CC1做触发 */
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;	/* 选择TIM2的CC2做触发 */
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	/* 数据右对齐,高位为0 */
		ADC_InitStructure.ADC_NbrOfChannel = 1;	/* 1个通道 */
		ADC_Init(ADC3, &ADC_InitStructure);

		/* ADC1 regular channels configuration */
		ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 1, ADC_SampleTime_1Cycles5);

		/* Enable ADC3 external trigger conversion */
		ADC_ExternalTrigConvCmd(ADC3, ENABLE);

		/* 使能 ADC3 */
		ADC_Cmd(ADC3, ENABLE);

		/* Enable ADC3 DMA */
		ADC_DMACmd(ADC3, ENABLE);

		/* 使能 ADC3 复位校准寄存器 */
		ADC_ResetCalibration(ADC3);
		/* 检测复位校准寄存器 */
		while(ADC_GetResetCalibrationStatus(ADC3));

		/* 开始 ADC3 校准 */
		ADC_StartCalibration(ADC3);
		/* 等待校准结束 */
		while(ADC_GetCalibrationStatus(ADC3));
	}
	#endif

	/* 配置采样触发定时器，使用TIM1 CC1 */
	SetSampRate(g_DSO->SampleFreq);	/* 修改采样频率(启动时100K) */
}

/*
*********************************************************************************************************
*	函 数 名: SetSampRate
*	功能说明: 修改采样频率
*	形    参: freq : 采样频率 单位Hz
*	返 回 值: 无
*********************************************************************************************************
*/
static void SetSampRate(uint32_t freq)
{
	/* 配置定时器2第2通道CC2, CC2作为ADC1的触发源 */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	uint16_t Period;

	TIM_Cmd(TIM1, DISABLE);

//	g_DSO->SampleRate = freq;	/* 将采样频率保存到全部变量中 */

	/*
	采样频率计算公式 ：
		period = 72000000 / freq ;
		
		1200 == 60KHz 采样频率
	*/
	Period = 72000000 / freq;		

	/* Enable TIM1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Period = Period;          
	TIM_TimeBaseStructure.TIM_Prescaler = 0;   /* 计数频率 = 72000 000 / 18 = 4000 000	 */
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* TIM1 channel1 configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;                
	TIM_OCInitStructure.TIM_Pulse = TIM_TimeBaseStructure.TIM_Period / 2; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;         
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);

	/* TIM1 counter enable */
	TIM_Cmd(TIM1, ENABLE);

	/* TIM1 main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  
}

/*
*********************************************************************************************************
*	函 数 名: DispHelp1
*	功能说明: 显示操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispHelp1(void)
{
	uint16_t y;
	uint16_t LineCap;
	FONT_T font;

	/* 分组框标题字体 */
	font.FontCode = FC_ST_16;
	font.BackColor = CL_BLUE;		/* 和背景色相同 */
	font.FrontColor = CL_WHITE;		/* 白色文字 */
	font.Space = 0;

	LCD_ClrScr(CL_BLUE);  		/* 清屏，背景蓝色 */

	y = 0;
	LineCap = 18; /* 行间距 */
	LCD_DispStr(20, y, "安富莱STM32F103ZE-EK开发板  www.armfly.com", &font);
	
	font.FrontColor = CL_YELLOW;		/* 黄色文字 */
	
	y += LineCap;	
	LCD_DispStr(30, y, "QQ:1295744630     Email:armfly@qq.com", &font);
	y += LineCap;

	y += LineCap;

	LCD_DispStr(30, y, "操作提示:", &font);
	y += LineCap;
	LCD_DispStr(50, y, "TAMPER键 = 切换通道焦点。CH1或CH2", &font);
	y += LineCap;
	LCD_DispStr(50, y, "WAKEUP键 = 显示帮助或退出帮助", &font);
	y += LineCap;
	LCD_DispStr(50, y, "USER  键 = 暂停或实时运行", &font);
	y += LineCap;
	LCD_DispStr(50, y, "摇杆上键 = 放大波形垂直幅度或向上移动", &font);
	y += LineCap;
	LCD_DispStr(50, y, "摇杆下键 = 缩小波形垂直幅度或向下移动", &font);
	y += LineCap;
	LCD_DispStr(50, y, "摇杆左键 = 水平展宽波形", &font);
	y += LineCap;
	LCD_DispStr(50, y, "摇杆右键 = 水平缩小波形", &font);
	y += LineCap;
	LCD_DispStr(50, y, "摇杆OK键 = 切换摇杆调节模式。幅度或位置", &font);
}

/*
*********************************************************************************************************
*	函 数 名: DsoHelp
*	功能说明: 显示操作提示的状态机
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DsoHelp(uint8_t *pMainStatus)
{
	uint8_t KeyCode;

	uint8_t fRefresh = 1;	/* LCD刷新标志 */
	uint8_t SubStatus = 0;

	while (*pMainStatus == S_HELP)
	{
		bsp_Idle();

		if (fRefresh)
		{
			fRefresh = 0;

			if (SubStatus == 0)
			{
				DispHelp1();
			}
		}

		/* 读取按键，大于0表示有键按下 */
		KeyCode = bsp_GetKey();
		if (KeyCode > 0)
		{
			/* 有键按下 */
			switch (KeyCode)
			{
				case KEY_DOWN_K2:				
					/* 退出,进入全速运行状态 */
					*pMainStatus = S_RUN;
					break;

				case JOY_DOWN_L:	/* 摇杆LEFT键按下 */
				case JOY_DOWN_R:	/* 摇杆RIGHT键按下 */
				case KEY_DOWN_K3:			
				case JOY_DOWN_OK:	/* 摇杆OK键 */
					/* 退出,进入全速运行状态 */
					*pMainStatus = S_EXIT;
					break;				

				case JOY_DOWN_U:		/* 摇杆UP键按下 */
					break;

				case JOY_DOWN_D:		/* 摇杆DOWN键按下 */
					break;
			
				default:
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispFrame
*	功能说明: 能：显示波形窗口的边框和刻度线
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFrame(void)
{
	uint16_t x, y;

	/* 绘制一个实线矩形框 x, y, h, w */
	LCD_DrawRect(9, 19, 202, 302, CL_WHITE);

	/* 绘制垂直刻度点 */
	for (x = 0; x < 13; x++)
	{
		for (y = 0; y < 41; y++)
		{
			LCD_PutPixel(10 + (x * 25), 20 + (y * 5), CL_WHITE);
		}
	}

	/* 绘制水平刻度点 */
	for (y = 0; y < 9; y++)
	{
		for (x = 0; x < 61; x++)
		{
			LCD_PutPixel(10 + (x * 5), 20 + (y * 25), CL_WHITE);
		}
	}

	/* 绘制垂直中心刻度点 */
	for (y = 0; y < 41; y++)
	{	 
		LCD_PutPixel(9 + (6 * 25), 20 + (y * 5), CL_WHITE);
		LCD_PutPixel(11 + (6 * 25), 20 + (y * 5), CL_WHITE);
	}

	/* 绘制水平中心刻度点 */
	for (x = 0; x < 61; x++)
	{	 
		LCD_PutPixel(10 + (x * 5), 19 + (4 * 25), CL_WHITE);
		LCD_PutPixel(10 + (x * 5), 21 + (4 * 25), CL_WHITE);
	}
}

/*
*********************************************************************************************************
*	函 数 名: DispButton
*	功能说明: 显示波形窗口右边的功能按钮（待扩展）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispButton(void)
{

}

/*
*********************************************************************************************************
*	函 数 名: DispCh1Wave
*	功能说明: 显示通道1波形
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispCh1Wave(void)
{
	int16_t i;		/* 有符号数 */
	//uint16_t pos;
	uint16_t *px;
	uint16_t *py;
	int16_t iTemp;

	/* 显示通道1电平标记 */
	{
		static uint16_t y = 70;
	
		LCD_DrawLine(1, y, 7, y, CL_BLUE);	 /* 选择蓝色 */

		y = g_DSO->Ch1VOffset;

		if (y < 20)
		{
			y = 20;
		}
		else if (y > 220)
		{
			y = 220;
		}
		LCD_DrawLine(1, y, 5, y, CL_YELLOW);
	}

//	if (s_DispFirst == 0)
//	{
//		s_DispFirst = 1;
//		LCD_ClrScr(CL_BLUE);  			/* 清屏，背景蓝色 */
//	}

	if (g_DSO->BufUsed == 0)
	{
		g_DSO->BufUsed = 1;
	}
	else
	{
		g_DSO->BufUsed = 0;
	}

	if (g_DSO->BufUsed == 0)
	{
		px = g_DSO->xCh1Buf1;
		py = g_DSO->yCh1Buf1;
	}
	else
	{
		px = g_DSO->xCh1Buf2;
		py = g_DSO->yCh1Buf2;
	}

	/* 计算当前最新的数据位置，向前递减400个样本 */
	//pos = SAMPLE_COUNT - DMA_GetCurrDataCounter(DMA1_Channel1);
	//pos = 0;

	for (i = 0; i < 300; i++)
	{
		px[i] = 10 + i;
		/* ADC = 2024 是BNC悬空输入时的ADC数值，统计多块板子获得的  */
		iTemp = g_DSO->Ch1VOffset + (int16_t)((2024 - g_DSO->Ch1Buf[i + 1]) * 10) / g_DSO->Ch1Attenuation;

		if (iTemp > 220)
		{
			iTemp = 220;
		}
		else if (iTemp < 20)
		{
			iTemp = 20;
		}
		py[i] = iTemp;
	}

	/* 清除上帧波形 */
	if (g_DSO->BufUsed == 0)
	{
		LCD_DrawPoints(g_DSO->xCh1Buf2, g_DSO->yCh1Buf2, 300, CL_BLUE);
	}
	else
	{
		LCD_DrawPoints(g_DSO->xCh1Buf1, g_DSO->yCh1Buf1, 300, CL_BLUE);
	}

	/* 显示更新的波形 */
	LCD_DrawPoints((uint16_t *)px, (uint16_t *)py, 300, CL_YELLOW);
}

/*
*********************************************************************************************************
*	函 数 名: DispCh2Wave
*	功能说明: 显示通道2波形
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispCh2Wave(void)
{
	int16_t i;		/* 有符号数 */
	//uint16_t pos;
	uint16_t *px;
	uint16_t *py;
	int16_t iTemp;

	/* 显示通道2电平标记 */
	{
		static uint16_t y = 170;

		LCD_DrawLine(1, y, 5, y, CL_BLUE);

		y = g_DSO->Ch2VOffset;

		if (y < 20)
		{
			y = 20;
		}
		else if (y > 220)
		{
			y = 220;
		}
		LCD_DrawLine(1, y, 5, y, CL_GREEN);
	}
		
	if (g_DSO->BufUsed == 0)
	{
		px = g_DSO->xCh2Buf1;
		py = g_DSO->yCh2Buf1;
	}
	else
	{
		px = g_DSO->xCh2Buf2;
		py = g_DSO->yCh2Buf2;
	}

	/* 计算当前最新的数据位置，向前递减400个样本 */
	//pos = SAMPLE_COUNT - DMA_GetCurrDataCounter(DMA2_Channel5);
	//pos = 0;

	for (i = 0; i < 300; i++)
	{
		px[i] = 10 + i;
		
		/* ADC = 2024 是BNC悬空输入时的ADC数值，统计多块板子获得的  */
		iTemp = g_DSO->Ch2VOffset + (int16_t)((2024 - g_DSO->Ch2Buf[i + 1]) * 10) / g_DSO->Ch2Attenuation;

		if (iTemp > 220)
		{
			iTemp = 220;
		}
		else if (iTemp < 20)
		{
			iTemp = 20;
		}
		py[i] = iTemp;
	}

	/* 清除上帧波形 */
	if (g_DSO->BufUsed == 0)
	{
		LCD_DrawPoints(g_DSO->xCh2Buf2, g_DSO->yCh2Buf2, 300, CL_BLUE);
	}
	else
	{
		LCD_DrawPoints(g_DSO->xCh2Buf1, g_DSO->yCh2Buf1, 300, CL_BLUE);
	}
	/* 显示更新的波形 */
	LCD_DrawPoints((uint16_t *)px, (uint16_t *)py, 300, CL_GREEN);
}

/*
*********************************************************************************************************
*	函 数 名: DispChInfo
*	功能说明: 显示通道信息
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispChInfo(void)
{
	char buf[32];   /* 字符显示缓冲区 */
	FONT_T font;

	/* 分组框标题字体 */
	font.FontCode = FC_ST_16;
	font.BackColor = CL_BLUE;		/* 和背景色相同 */
	font.FrontColor = CL_WHITE;		/* 白色文字 */
	font.Space = 0;	

	/* 显示示波器程序版本 */
	LCD_DispStr(10, 2, DSO_VER, &font);

	/* 显示通道1信息 */
	if (g_DSO->Ch1DC == 1)
	{
		strcpy(buf, "CH1 DC ");
	}
	else
	{
		strcpy(buf, "CH1 AC ");
	}

	if (g_DSO->Ch1VScale >= 1000)
	{
		sprintf(&buf[7], "%d.00V", g_DSO->Ch1VScale / 1000);
	}
	else
	{
		sprintf(&buf[7], "%dmV", g_DSO->Ch1VScale);
	}

	if (g_DSO->ActiveCH == 1)
	{
		font.BackColor = CL_YELLOW;		/* 黄色 */
		font.FrontColor = CL_MAGENTA;	/* 紫色 */	
	}
	else
	{
		font.BackColor = CL_YELLOW;		/* 黄色 */
		font.FrontColor = CL_BLUE;		/* 蓝色 */	
	}

	LCD_DispStr(10, 224, buf, &font);

	/* 显示通道2信息 */
	font.FrontColor = CL_RED; /* CH2 红色 */
	if (g_DSO->Ch2DC == 1)
	{
		strcpy(buf, "CH2 DC ");
	}
	else
	{
		strcpy(buf, "CH2 AC ");
	}

	if (g_DSO->Ch2VScale >= 1000)
	{
		sprintf(&buf[7], "%d.00V", g_DSO->Ch2VScale / 1000);
	}
	else
	{
		sprintf(&buf[7], "%dmV", g_DSO->Ch2VScale);
	}
	if (g_DSO->ActiveCH == 2)
	{
		font.BackColor = CL_GREEN;		/* 绿色 */
		font.FrontColor = CL_MAGENTA;	/* 紫色 */			
	}
	else
	{
		font.BackColor = CL_GREEN;		/* 绿色 */
		font.FrontColor = CL_BLUE;		/* 紫色 */			
	}
	LCD_DispStr(120, 224, buf, &font);

	/* 显示时基 */
	font.FrontColor = CL_WHITE;		/* 白色 */		
	font.BackColor = CL_BLUE;		/* 蓝色 */
		
	if (g_DSO->TimeBase < 1000)
	{
		sprintf(buf, "Time %3dus", g_DSO->TimeBase);
	}
	else if (g_DSO->TimeBase < 1000000)
	{
		sprintf(buf, "Time %3dms", g_DSO->TimeBase / 1000);
	}
	else
	{
		sprintf(buf, "Time %3ds ", g_DSO->TimeBase / 1000000);	
	}
	LCD_DispStr(230, 224,  buf, &font);


	/* 显示调节模式 */
	font.FrontColor = CL_WHITE;		/* 白字 */		
	font.BackColor = CL_BLUE;		/* 蓝底 */
	
	if (g_DSO->AdjustMode == 1)
	{
		LCD_DispStr(245, 2, "调节位置", &font);
	}
	else
	{
		LCD_DispStr(245, 2, "调节幅度", &font);
	}

	sprintf(buf, "采样频率:%7dHz",	g_DSO->SampleFreq);
	LCD_DispStr(75, 2, buf, &font);
}

/*
*********************************************************************************************************
*	函 数 名: DispDSO
*	功能说明: DispDSO
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispDSO(void)
{
	DispButton();

	DispFrame();	/* 绘制刻度框 */
	DispChInfo();	/* 显示通道信息(幅度，时间档位) */

	DispCh1Wave();	/* 显示波形1 */	
	DispCh2Wave();	/* 显示波形2 */	
}

/*
*********************************************************************************************************
*	函 数 名: InitDsoParam
*	功能说明: 初始化全局参数变量
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitDsoParam(void)
{
	g_DSO->Ch1Attenuation = 23;	/* 设置缺省衰减系数 */
	g_DSO->Ch2Attenuation = 23;	/* 设置缺省衰减系数 */
	g_DSO->Ch1VScale = 1000;	/* 缺省是1V */
	g_DSO->Ch2VScale = 1000;	/* 缺省是1V */

	g_DSO->Ch1VOffset = 70; 	/* 通道1 GND线位置 */
	g_DSO->Ch2VOffset = 170; /* 通道2 GND线位置 */

	g_DSO->ActiveCH = 1;		/* 缺省是CH1 */
	g_DSO->AdjustMode = 1;	/* 缺省是调节垂直偏移， 可以切换到2调节幅度 */

	g_DSO->HoldEn = 0;

	g_DSO->TimeBaseId = 0;									 
	g_DSO->TimeBase = 	g_TimeTable[g_DSO->TimeBaseId];
	g_DSO->SampleFreq = 25000000 / g_DSO->TimeBase;


	g_DSO->Ch1AttId = 2;
	g_DSO->Ch1Attenuation = g_AttTable[g_DSO->Ch1AttId][0];
	g_DSO->Ch1VScale =  g_AttTable[g_DSO->Ch1AttId][1];

	g_DSO->Ch2AttId = 2;
	g_DSO->Ch2Attenuation = g_AttTable[g_DSO->Ch2AttId][0];
	g_DSO->Ch2VScale =  g_AttTable[g_DSO->Ch2AttId][1];

}

/*******************************************************************************
*	函数名: IncSampleFreq
*	输  入:	无
*	输  出:	无
*	功  能：增加采样频率，按 1-2-5 
*	
*	时间轴分度（每1个大格的时长)
		20us 	      1250000
		50us 		   500000
		100us		   250000
		200us		   125000
		500us		    50000
		1ms				 2500
		2ms				 1250
		5ms				  500 
		10ms			  250
		20ms			  125
		50ms			   50
		100ms			   25

//		200ms			   12.5
//		500ms			    5

	g_DSO->TimeScale = 25000000 / g_DSO->SampleRate;
*/

/*
*********************************************************************************************************
*	函 数 名: IncSampleFreq
*	功能说明: 增加采样频率，按 1-2-5 
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	时间轴分度（每1个大格的时长)
		20us 	      1250000
		50us 		   500000
		100us		   250000
		200us		   125000
		500us		    50000
		1ms				 2500
		2ms				 1250
		5ms				  500 
		10ms			  250
		20ms			  125
		50ms			   50
		100ms			   25

//		200ms			   12.5
//		500ms			    5

	g_DSO->TimeScale = 25000000 / g_DSO->SampleRate;
*/
static void IncSampleFreq(void)
{
	if (g_DSO->TimeBaseId < (sizeof(g_TimeTable) / 4) - 1)
	{
		g_DSO->TimeBaseId++;
	}
													 
	g_DSO->TimeBase = 	g_TimeTable[g_DSO->TimeBaseId];
	g_DSO->SampleFreq = 25000000 / g_DSO->TimeBase;

	SetSampRate(g_DSO->SampleFreq);	/* 改变采样频率 */
}

/*
*********************************************************************************************************
*	函 数 名: DecSampleFreq
*	功能说明: 降低采样频率，按 1-2-5 
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DecSampleFreq(void)
{
	/*	时间轴分度（每1个大格的时长)
		20us 	      1250000
		50us 		   500000
		100us		   250000
		200us		   125000
		500us		    50000
		1ms				 2500
		2ms				 1250
		5ms				  500 
		10ms			  250
		20ms			  125
		50ms			   50
		100ms			   25

	*/
	if (g_DSO->TimeBaseId > 0)
	{
		g_DSO->TimeBaseId--;
	}
													 
	g_DSO->TimeBase = g_TimeTable[g_DSO->TimeBaseId];
	g_DSO->SampleFreq = 25000000 / g_DSO->TimeBase;

	SetSampRate(g_DSO->SampleFreq);	/* 改变采样频率 */
}

/*
*********************************************************************************************************
*	函 数 名: AdjustAtt
*	功能说明: 调节电压衰减档位，按 1-2-5 
*	形    参: ch   : 通道号，1或2
*			  mode : 0 降低， 1增加
*	返 回 值: 无
*********************************************************************************************************
*/
static void AdjustAtt(uint8_t ch, uint8_t mode)
{

	if (ch == 1)
	{
		if (mode == 0) 	/* 降低 */
		{
			if (g_DSO->Ch1AttId > 0)
			{
				g_DSO->Ch1AttId--;
			}
		}
		else		/* 增加 */
		{
			if (g_DSO->Ch1AttId < ATT_COUNT - 1)
			{
				g_DSO->Ch1AttId++;
			}
		}

		g_DSO->Ch1Attenuation = g_AttTable[g_DSO->Ch1AttId][0];
		g_DSO->Ch1VScale =  g_AttTable[g_DSO->Ch1AttId][1];
	}
	else if (ch == 2)
	{
		if (mode == 0) 	/* 降低 */
		{
			if (g_DSO->Ch2AttId > 0)
			{
				g_DSO->Ch2AttId--;
			}
		}
		else		/* 增加 */
		{
			if (g_DSO->Ch2AttId < ATT_COUNT - 1)
			{
				g_DSO->Ch2AttId++;
			}
		}
		g_DSO->Ch2Attenuation = g_AttTable[g_DSO->Ch2AttId][0];
		g_DSO->Ch2VScale =  g_AttTable[g_DSO->Ch2AttId][1];
	}
}

/*
*********************************************************************************************************
*	函 数 名: AdjustAtt
*	功能说明: 暂停ADC采样
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void StopADC(void)
{
	TIM_Cmd(TIM1, DISABLE);

	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, DISABLE);
	
	/* Enable DMA2 channel5 */
	DMA_Cmd(DMA2_Channel5, DISABLE);
}

/*
*********************************************************************************************************
*	函 数 名: StartADC
*	功能说明: 启动ADC采样
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void StartADC(void)
{
#if 1
	DMA_InitTypeDef DMA_InitStructure;

	DMA_DeInit(DMA1_Channel1);		/* 复位DMA1寄存器到缺省状态 */
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	/* 选择ADC1的数据寄存器作为源 */
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_DSO->Ch1Buf;	/* 目标地址 */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* 设置DMA传输方向，外设(ADC)作为源 */
	DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* 设置缓冲区大小 */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* 外设地址不自增 */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* 存储器地址需要自增 */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* 选择外设传输单位：16bit */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* 选择内存传输单位：16bit */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* 无需循环模式   */
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	/* 选择DMA优先级 */
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA传输类型，不是内存到内存 */
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	DMA_DeInit(DMA2_Channel5);		/* 复位DMA2寄存器到缺省状态 */
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC3_DR_Address;	/* 选择ADC1的数据寄存器作为源 */
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_DSO->Ch2Buf;	/* 目标地址 */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* 设置DMA传输方向，外设(ADC)作为源 */
	DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* 设置缓冲区大小 */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* 外设地址不自增 */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* 存储器地址需要自增 */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* 选择外设传输单位：16bit */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* 选择内存传输单位：16bit */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* 无需循环模式   */
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	/* 选择DMA优先级 */
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA传输类型，不是内存到内存 */
	DMA_Init(DMA2_Channel5, &DMA_InitStructure);
#endif

	/* Enable DMA2 channel5 */
	DMA_Cmd(DMA1_Channel1, ENABLE);

	/* Enable DMA2 channel5 */
	DMA_Cmd(DMA2_Channel5, ENABLE);

	TIM_Cmd(TIM1, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: DsoRun
*	功能说明: DSO运行状态
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DsoRun(uint8_t *pMainStatus)
{
	uint8_t KeyCode;
	uint8_t fRefresh = 1;	/* LCD刷新标志 */
	FormDSO_T form;	
	uint8_t ucTouch;
	int16_t tpX, tpY;	

	InitDsoParam();	/* 初始化示波器参数 */

	InitDSO();		/* 配置示波器用到的外设：ADC,TIM,DMA等 */

	LCD_ClrScr(CL_BLUE);  			/* 清屏，背景蓝色 */
	
	FormDSO = &form;
	
	/* 绘制一个按钮 */
	{
		/* 按钮字体 */
		FormDSO->FontBtn.FontCode = FC_ST_16;
		FormDSO->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
		FormDSO->FontBtn.FrontColor = CL_BLACK;
		FormDSO->FontBtn.Space = 0;
		
		/* 按钮 */
		FormDSO->BtnRet.Left = BTN_RET_X;
		FormDSO->BtnRet.Top = BTN_RET_Y;
		FormDSO->BtnRet.Height = BTN_RET_H;
		FormDSO->BtnRet.Width = BTN_RET_W;
		FormDSO->BtnRet.pCaption = BTN_RET_T;
		FormDSO->BtnRet.Font = &FormDSO->FontBtn;
		FormDSO->BtnRet.Focus = 0;
		
		LCD_DrawButton(&FormDSO->BtnRet);
	}

	AD9833_SetWaveFreq(100000);		/* 频率 = 10KHz, 单位是0.1Hz */
	
	AD9833_SetWaveFreq(100000);		/* 频率 = 10KHz, 单位是0.1Hz */
	AD9833_SelectWave(SINE_WAVE);	/* 正弦波 */	
	
	bsp_StartTimer(1, 150);		/* 启动定时器1，100ms刷新1次 */
	while (*pMainStatus == S_RUN)
	{
		bsp_Idle();

		if (fRefresh)
		{
			fRefresh = 0;

			//StopADC();	/* 暂停采样 */
			if (g_DSO->HoldEn == 1)
			{
				DispDSO();
			}
			//StartADC();	/* 开始采样 */
		}

		if (bsp_CheckTimer(1))
		{
			bsp_StartTimer(1, 150);		/* 启动定时器1，100ms刷新1次 */

			/* 运行状态。每隔100ms刷新1次波形 */
			if (g_DSO->HoldEn == 0)
			{
				StopADC();	/* 暂停采样 */
				DispDSO();
				StartADC();	/* 开始采样 */
			}
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
					if (LCD_ButtonTouchDown(&FormDSO->BtnRet, tpX, tpY))
					{
						// *pMainStatus = S_EXIT;
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (LCD_ButtonTouchRelease(&FormDSO->BtnRet, tpX, tpY))
					{
						*pMainStatus = S_EXIT;
					}
					break;
			}
		}
		
		/* 读取按键，大于0表示有键按下 */
		KeyCode = bsp_GetKey();
		if (KeyCode > 0)
		{
			/* 有键按下 */
			switch (KeyCode)
			{
				case  KEY_DOWN_K1:	/* TAMPER 键，通道选择(CH1或CH2) */
					if (g_DSO->ActiveCH == 1)
					{
						g_DSO->ActiveCH = 2;
					}
					else
					{
						g_DSO->ActiveCH = 1;
					}
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case  KEY_DOWN_K2:	/* WAKEUP 键, 调节模式选择(幅度或者垂直偏移) */
					/* 退出,进入全速运行状态 */
					*pMainStatus = S_HELP;
					break;

				case  KEY_DOWN_K3:	/* USER 键 */
					if (g_DSO->HoldEn == 0)
					{
						g_DSO->HoldEn = 1;

						/* 保存暂停时的时基,为了水平扩展用 */
						g_DSO->TimeBaseIdHold = g_DSO->TimeBaseId;	

						StopADC();
					}
					else
					{
						g_DSO->HoldEn = 0;
						StartADC();				
					}
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case JOY_DOWN_L:	/* 摇杆LEFT键按下 */
					if (g_DSO->HoldEn == 0)
					{
						DecSampleFreq();	/* 递减采样频率 */
						fRefresh = 1;		/* 请求刷新LCD */
					}
					else
					{
						; /* 波形水平移动，待完善 */
					}
					break;

				case JOY_DOWN_R:	/* 摇杆RIGHT键按下 */
					if (g_DSO->HoldEn == 0)
					{
						IncSampleFreq();  	/* 递增采样频率 */					
						fRefresh = 1;		/* 请求刷新LCD */
					}
					else
					{
						; /* 波形水平移动，待完善 */
					}
					break;

				case  JOY_DOWN_OK:	/* 摇杆OK键 */
					if (g_DSO->AdjustMode == 0)
					{
						g_DSO->AdjustMode = 1;
					}
					else
					{
						g_DSO->AdjustMode = 0;
					}
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case JOY_DOWN_U:	/* 摇杆UP键按下 */
					if (g_DSO->ActiveCH == 1) 	/* 当前激活的是CH1 */
					{
						if (g_DSO->AdjustMode == 0)	/* 调节幅度放大倍数 */
						{
							AdjustAtt(1, 1);
						}
						else 	/* 调节上下偏移 */
						{
							g_DSO->Ch1VOffset -= 5;
						}
					}
					else	/* 当前激活的是CH2 */
					{
						if (g_DSO->AdjustMode == 0)	/* 调节幅度放大倍数 */
						{
							AdjustAtt(2, 1);
						}
						else 	/* 调节上下偏移 */
						{
							g_DSO->Ch2VOffset -= 5;
						}
					}
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				case JOY_DOWN_D:		/* 摇杆DOWN键按下 */
					if (g_DSO->ActiveCH == 1) 	/* 当前激活的是CH1 */
					{
						if (g_DSO->AdjustMode == 0)	/* 调节幅度放大倍数 */
						{
							AdjustAtt(1, 0);
						}
						else 	/* 调节上下偏移 */
						{
							g_DSO->Ch1VOffset += 5;
						}
					}
					else	/* 当前激活的是CH2 */
					{
						if (g_DSO->AdjustMode == 0)	/* 调节幅度放大倍数 */
						{
							AdjustAtt(2, 0);
						}
						else 	/* 调节上下偏移 */
						{
							g_DSO->Ch2VOffset += 5;
						}
					}
					fRefresh = 1;		/* 请求刷新LCD */
					break;

				default:
					break;
			}
		}
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

/*
*********************************************************************************************************
*
*	ģ������ : ˫ͨ��ʾ��������
*	�ļ����� : form_dso.c
*	��    �� : V1.0
*	˵    �� : ʹ��STM32�ڲ�ADC�������Ρ�CH1 = PC0�� CH2 = PC1
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2015-06-23  armfly  ��ʽ����
*		V1.1    2015-08-07  armfly  ʹ�ö�ջ�����ߴ�����ݻ����������ȫ�ֱ�����
*
*	Copyright (C), 2015-2016, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_dso.h"
#include "WaveMaker.h"

#define DSO_VER	"V0.6"		  /* ��ǰ�汾 */

/* ������״̬�� */
enum
{
	S_HELP		= 0,	/* ����ҳ�� */
	S_RUN		= 1,	/* �ɼ�״̬ */

	S_EXIT		= 3
};

/* ʱ��ֶȱ� g_DSO->TimeBaseId��Ϊ��������  */
const uint32_t g_TimeTable[] =
{
	20, 	50, 	100, 
	200,	500,	1000,
	2000,	5000,	10000,
	20000,	50000,	100000,
	200000,	500000,	1000000		
};

/* ˥��������  


���㹫ʽ��
  iTemp = g_DSO->Ch1VOffset + (int16_t)((2024 - g_DSO->Ch1Buf[i + 1]) * 10) / g_DSO->Ch1Attenuation;

	g_DSO->Ch1Attenuation ��ADCֵ������֮��ı��ʵ�10����

	1V ��λʱ:  ADC = 4096 / 5.0 = 819�� ���� = 25
	g_DSO->Ch1Attenuation = 819 / 25 = 32.76

*/
#define ATT_COUNT	6
#define Y_RATE  327
const uint32_t g_AttTable[ATT_COUNT][2] =
{
	/* {����*0.1, ÿ����ѹ}  1:1 */
	{Y_RATE * 5,	5000},
	{Y_RATE * 2,	2000},
	{Y_RATE,	1000},   /*��ȱʡֵ 1���25���أ���Ӧ1V.��*/
	{Y_RATE / 2,	500},
	{Y_RATE / 5,	200},
	{Y_RATE / 10,	100},
};

static void DsoHelp(uint8_t *pMainStatus);
static void DsoRun(uint8_t *pMainStatus);

/*
	������STM32-V4 ���߷��䣺

	ʾ����ͨ��1
		PC0 = ADC1_IN10

	ʾ����ͨ��2
		PC1 = ADC2_IN12
*/

/* ��ť */
/* ���ذ�ť������(��Ļ���½�) */
#define BTN_RET_H	32
#define BTN_RET_W	80
#define	BTN_RET_X	(g_LcdWidth - BTN_RET_W - 8)
#define	BTN_RET_Y	(g_LcdHeight - BTN_RET_H - 4)
#define	BTN_RET_T	"����"

DSO_T *g_DSO;	/* ȫ�ֱ�������һ���ṹ�� */

/* �������ṹ */
typedef struct
{
	FONT_T FontBtn;		/* ��ť������ */
	
	BUTTON_T BtnRet;
}FormDSO_T;

FormDSO_T *FormDSO;

/*
*********************************************************************************************************
*	�� �� ��: DsoMain
*	����˵��: ʾ��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DsoMain(void)
{
	uint8_t MainStatus = S_RUN;		/* ����ִ��״̬ */
	DSO_T tDSO;			/* �ܴ��һ������������ڶ�ջ����Լȫ�ֱ����ռ� */
	
	memset(&tDSO, 0, sizeof(tDSO));
	g_DSO = &tDSO;

	InitWaveMaker();	/* ʹ��STM32�ڲ�DAC������Ҳ� */
	
	bsp_InitAD9833();				/* ʹ���ⲿAD9833ģ��������ǲ� */

	
	/* ����������ѭ���� */
	while (1)
	{
		switch (MainStatus)
		{
			case S_HELP:
				DsoHelp(&MainStatus);		/* ��ʾ���� */
				break;

			case S_RUN:
				DsoRun(&MainStatus);		/* ȫ�ٲɼ���ʵʱ��ʾ */
				break;

			case S_EXIT:
				AD9833_SelectWave(NONE_WAVE);	/* �ر���� */
				return;
			
			default:
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: InitDSO
*	����˵��: ��ʾ����ͨ��1���г�ʼ�����á���Ҫ���GPIO�����á�ADC�����á���ʱ���������Լ�DMA�����á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitDSO(void)
{
	/* ����GPIO.  */
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* ��GPIO_C �� AFIO ��ʱ�� */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC	| RCC_APB2Periph_AFIO, ENABLE);

		/* ����PC0Ϊģ������ģʽ */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
	}


	/* ����DMA1_1 ����CH1 */
	#if 1
	{
		DMA_InitTypeDef DMA_InitStructure;

		/* Enable DMA1 clock */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		DMA_DeInit(DMA1_Channel1);		/* ��λDMA1�Ĵ�����ȱʡ״̬ */
		DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	/* ѡ��ADC1�����ݼĴ�����ΪԴ */
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_DSO->Ch1Buf;	/* Ŀ���ַ */
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* ����DMA���䷽������(ADC)��ΪԴ */
		DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* ���û�������С */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* �����ַ������ */
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* �洢����ַ��Ҫ���� */
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* ѡ�����贫�䵥λ��16bit */
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* ѡ���ڴ洫�䵥λ��16bit */
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* ����ѭ��ģʽ   */
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;	/* ѡ��DMA���ȼ� */
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA�������ͣ������ڴ浽�ڴ� */
		DMA_Init(DMA1_Channel1, &DMA_InitStructure);

		/* Enable DMA1 channel1 */
		DMA_Cmd(DMA1_Channel1, ENABLE);
	}
	#endif

	/* ����DMA2_5 ����CH2 */
	#if 1
	{
		DMA_InitTypeDef DMA_InitStructure;

		/* Enable DMA1 clock */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

		DMA_DeInit(DMA2_Channel5);		/* ��λDMA2�Ĵ�����ȱʡ״̬ */
		DMA_InitStructure.DMA_PeripheralBaseAddr = ADC3_DR_Address;	/* ѡ��ADC3�����ݼĴ�����ΪԴ */
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_DSO->Ch2Buf;	/* Ŀ���ַ */
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* ����DMA���䷽������(ADC)��ΪԴ */
		DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* ���û�������С */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* �����ַ������ */
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* �洢����ַ��Ҫ���� */
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* ѡ�����贫�䵥λ��16bit */
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* ѡ���ڴ洫�䵥λ��16bit */
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* ����ѭ��ģʽ   */
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;	/* ѡ��DMA���ȼ� */
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA�������ͣ������ڴ浽�ڴ� */
		DMA_Init(DMA2_Channel5, &DMA_InitStructure);

		/* Enable DMA2 channel5 */
		DMA_Cmd(DMA2_Channel5, ENABLE);
	}
	#endif

	/* ����ADC1  */
	
	 /* ADCCLK = PCLK2/2 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);     //ADCCLK = PCLK2/4 = 18MHz

	#if 1
	{
		ADC_InitTypeDef	ADC_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		/* ����ת����ֹ */
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;	/* ѡ��TIM1��CC3������ */
		//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;	/* ѡ��TIM2��CC2������ */

		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	/* �����Ҷ���,��λΪ0 */
		ADC_InitStructure.ADC_NbrOfChannel = 1;	/* 1��ͨ�� */
		ADC_Init(ADC1, &ADC_InitStructure);

		/* ADC1 regular channels configuration */
		ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_1Cycles5);

		/* Enable ADC1 external trigger conversion */
		ADC_ExternalTrigConvCmd(ADC1, ENABLE);

		/* ʹ�� ADC1 */
		ADC_Cmd(ADC1, ENABLE);

		/* Enable ADC1 DMA */
		ADC_DMACmd(ADC1, ENABLE);

		/* ʹ�� ADC1 ��λУ׼�Ĵ��� */
		ADC_ResetCalibration(ADC1);
		/* ��⸴λУ׼�Ĵ��� */
		while(ADC_GetResetCalibrationStatus(ADC1));

		/* ��ʼ ADC1 У׼ */
		ADC_StartCalibration(ADC1);
		/* �ȴ�У׼���� */
		while(ADC_GetCalibrationStatus(ADC1));
	}
	#endif

	/* ����ADC3  */
	#if 1
	{
		ADC_InitTypeDef	ADC_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		/* ����ת����ֹ */
		//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;	/* ѡ��TIM1��CC1������ */
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;	/* ѡ��TIM2��CC2������ */
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	/* �����Ҷ���,��λΪ0 */
		ADC_InitStructure.ADC_NbrOfChannel = 1;	/* 1��ͨ�� */
		ADC_Init(ADC3, &ADC_InitStructure);

		/* ADC1 regular channels configuration */
		ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 1, ADC_SampleTime_1Cycles5);

		/* Enable ADC3 external trigger conversion */
		ADC_ExternalTrigConvCmd(ADC3, ENABLE);

		/* ʹ�� ADC3 */
		ADC_Cmd(ADC3, ENABLE);

		/* Enable ADC3 DMA */
		ADC_DMACmd(ADC3, ENABLE);

		/* ʹ�� ADC3 ��λУ׼�Ĵ��� */
		ADC_ResetCalibration(ADC3);
		/* ��⸴λУ׼�Ĵ��� */
		while(ADC_GetResetCalibrationStatus(ADC3));

		/* ��ʼ ADC3 У׼ */
		ADC_StartCalibration(ADC3);
		/* �ȴ�У׼���� */
		while(ADC_GetCalibrationStatus(ADC3));
	}
	#endif

	/* ���ò���������ʱ����ʹ��TIM1 CC1 */
	SetSampRate(g_DSO->SampleFreq);	/* �޸Ĳ���Ƶ��(����ʱ100K) */
}

/*
*********************************************************************************************************
*	�� �� ��: SetSampRate
*	����˵��: �޸Ĳ���Ƶ��
*	��    ��: freq : ����Ƶ�� ��λHz
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SetSampRate(uint32_t freq)
{
	/* ���ö�ʱ��2��2ͨ��CC2, CC2��ΪADC1�Ĵ���Դ */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	uint16_t Period;

	TIM_Cmd(TIM1, DISABLE);

//	g_DSO->SampleRate = freq;	/* ������Ƶ�ʱ��浽ȫ�������� */

	/*
	����Ƶ�ʼ��㹫ʽ ��
		period = 72000000 / freq ;
		
		1200 == 60KHz ����Ƶ��
	*/
	Period = 72000000 / freq;		

	/* Enable TIM1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Period = Period;          
	TIM_TimeBaseStructure.TIM_Prescaler = 0;   /* ����Ƶ�� = 72000 000 / 18 = 4000 000	 */
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
*	�� �� ��: DispHelp1
*	����˵��: ��ʾ������ʾ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispHelp1(void)
{
	uint16_t y;
	uint16_t LineCap;
	FONT_T font;

	/* ������������ */
	font.FontCode = FC_ST_16;
	font.BackColor = CL_BLUE;		/* �ͱ���ɫ��ͬ */
	font.FrontColor = CL_WHITE;		/* ��ɫ���� */
	font.Space = 0;

	LCD_ClrScr(CL_BLUE);  		/* ������������ɫ */

	y = 0;
	LineCap = 18; /* �м�� */
	LCD_DispStr(20, y, "������STM32F103ZE-EK������  www.armfly.com", &font);
	
	font.FrontColor = CL_YELLOW;		/* ��ɫ���� */
	
	y += LineCap;	
	LCD_DispStr(30, y, "QQ:1295744630     Email:armfly@qq.com", &font);
	y += LineCap;

	y += LineCap;

	LCD_DispStr(30, y, "������ʾ:", &font);
	y += LineCap;
	LCD_DispStr(50, y, "TAMPER�� = �л�ͨ�����㡣CH1��CH2", &font);
	y += LineCap;
	LCD_DispStr(50, y, "WAKEUP�� = ��ʾ�������˳�����", &font);
	y += LineCap;
	LCD_DispStr(50, y, "USER  �� = ��ͣ��ʵʱ����", &font);
	y += LineCap;
	LCD_DispStr(50, y, "ҡ���ϼ� = �Ŵ��δ�ֱ���Ȼ������ƶ�", &font);
	y += LineCap;
	LCD_DispStr(50, y, "ҡ���¼� = ��С���δ�ֱ���Ȼ������ƶ�", &font);
	y += LineCap;
	LCD_DispStr(50, y, "ҡ����� = ˮƽչ����", &font);
	y += LineCap;
	LCD_DispStr(50, y, "ҡ���Ҽ� = ˮƽ��С����", &font);
	y += LineCap;
	LCD_DispStr(50, y, "ҡ��OK�� = �л�ҡ�˵���ģʽ�����Ȼ�λ��", &font);
}

/*
*********************************************************************************************************
*	�� �� ��: DsoHelp
*	����˵��: ��ʾ������ʾ��״̬��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DsoHelp(uint8_t *pMainStatus)
{
	uint8_t KeyCode;

	uint8_t fRefresh = 1;	/* LCDˢ�±�־ */
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

		/* ��ȡ����������0��ʾ�м����� */
		KeyCode = bsp_GetKey();
		if (KeyCode > 0)
		{
			/* �м����� */
			switch (KeyCode)
			{
				case KEY_DOWN_K2:				
					/* �˳�,����ȫ������״̬ */
					*pMainStatus = S_RUN;
					break;

				case JOY_DOWN_L:	/* ҡ��LEFT������ */
				case JOY_DOWN_R:	/* ҡ��RIGHT������ */
				case KEY_DOWN_K3:			
				case JOY_DOWN_OK:	/* ҡ��OK�� */
					/* �˳�,����ȫ������״̬ */
					*pMainStatus = S_EXIT;
					break;				

				case JOY_DOWN_U:		/* ҡ��UP������ */
					break;

				case JOY_DOWN_D:		/* ҡ��DOWN������ */
					break;
			
				default:
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DispFrame
*	����˵��: �ܣ���ʾ���δ��ڵı߿�Ϳ̶���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispFrame(void)
{
	uint16_t x, y;

	/* ����һ��ʵ�߾��ο� x, y, h, w */
	LCD_DrawRect(9, 19, 202, 302, CL_WHITE);

	/* ���ƴ�ֱ�̶ȵ� */
	for (x = 0; x < 13; x++)
	{
		for (y = 0; y < 41; y++)
		{
			LCD_PutPixel(10 + (x * 25), 20 + (y * 5), CL_WHITE);
		}
	}

	/* ����ˮƽ�̶ȵ� */
	for (y = 0; y < 9; y++)
	{
		for (x = 0; x < 61; x++)
		{
			LCD_PutPixel(10 + (x * 5), 20 + (y * 25), CL_WHITE);
		}
	}

	/* ���ƴ�ֱ���Ŀ̶ȵ� */
	for (y = 0; y < 41; y++)
	{	 
		LCD_PutPixel(9 + (6 * 25), 20 + (y * 5), CL_WHITE);
		LCD_PutPixel(11 + (6 * 25), 20 + (y * 5), CL_WHITE);
	}

	/* ����ˮƽ���Ŀ̶ȵ� */
	for (x = 0; x < 61; x++)
	{	 
		LCD_PutPixel(10 + (x * 5), 19 + (4 * 25), CL_WHITE);
		LCD_PutPixel(10 + (x * 5), 21 + (4 * 25), CL_WHITE);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DispButton
*	����˵��: ��ʾ���δ����ұߵĹ��ܰ�ť������չ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispButton(void)
{

}

/*
*********************************************************************************************************
*	�� �� ��: DispCh1Wave
*	����˵��: ��ʾͨ��1����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispCh1Wave(void)
{
	int16_t i;		/* �з����� */
	//uint16_t pos;
	uint16_t *px;
	uint16_t *py;
	int16_t iTemp;

	/* ��ʾͨ��1��ƽ��� */
	{
		static uint16_t y = 70;
	
		LCD_DrawLine(1, y, 7, y, CL_BLUE);	 /* ѡ����ɫ */

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
//		LCD_ClrScr(CL_BLUE);  			/* ������������ɫ */
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

	/* ���㵱ǰ���µ�����λ�ã���ǰ�ݼ�400������ */
	//pos = SAMPLE_COUNT - DMA_GetCurrDataCounter(DMA1_Channel1);
	//pos = 0;

	for (i = 0; i < 300; i++)
	{
		px[i] = 10 + i;
		/* ADC = 2024 ��BNC��������ʱ��ADC��ֵ��ͳ�ƶ����ӻ�õ�  */
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

	/* �����֡���� */
	if (g_DSO->BufUsed == 0)
	{
		LCD_DrawPoints(g_DSO->xCh1Buf2, g_DSO->yCh1Buf2, 300, CL_BLUE);
	}
	else
	{
		LCD_DrawPoints(g_DSO->xCh1Buf1, g_DSO->yCh1Buf1, 300, CL_BLUE);
	}

	/* ��ʾ���µĲ��� */
	LCD_DrawPoints((uint16_t *)px, (uint16_t *)py, 300, CL_YELLOW);
}

/*
*********************************************************************************************************
*	�� �� ��: DispCh2Wave
*	����˵��: ��ʾͨ��2����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispCh2Wave(void)
{
	int16_t i;		/* �з����� */
	//uint16_t pos;
	uint16_t *px;
	uint16_t *py;
	int16_t iTemp;

	/* ��ʾͨ��2��ƽ��� */
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

	/* ���㵱ǰ���µ�����λ�ã���ǰ�ݼ�400������ */
	//pos = SAMPLE_COUNT - DMA_GetCurrDataCounter(DMA2_Channel5);
	//pos = 0;

	for (i = 0; i < 300; i++)
	{
		px[i] = 10 + i;
		
		/* ADC = 2024 ��BNC��������ʱ��ADC��ֵ��ͳ�ƶ����ӻ�õ�  */
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

	/* �����֡���� */
	if (g_DSO->BufUsed == 0)
	{
		LCD_DrawPoints(g_DSO->xCh2Buf2, g_DSO->yCh2Buf2, 300, CL_BLUE);
	}
	else
	{
		LCD_DrawPoints(g_DSO->xCh2Buf1, g_DSO->yCh2Buf1, 300, CL_BLUE);
	}
	/* ��ʾ���µĲ��� */
	LCD_DrawPoints((uint16_t *)px, (uint16_t *)py, 300, CL_GREEN);
}

/*
*********************************************************************************************************
*	�� �� ��: DispChInfo
*	����˵��: ��ʾͨ����Ϣ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispChInfo(void)
{
	char buf[32];   /* �ַ���ʾ������ */
	FONT_T font;

	/* ������������ */
	font.FontCode = FC_ST_16;
	font.BackColor = CL_BLUE;		/* �ͱ���ɫ��ͬ */
	font.FrontColor = CL_WHITE;		/* ��ɫ���� */
	font.Space = 0;	

	/* ��ʾʾ��������汾 */
	LCD_DispStr(10, 2, DSO_VER, &font);

	/* ��ʾͨ��1��Ϣ */
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
		font.BackColor = CL_YELLOW;		/* ��ɫ */
		font.FrontColor = CL_MAGENTA;	/* ��ɫ */	
	}
	else
	{
		font.BackColor = CL_YELLOW;		/* ��ɫ */
		font.FrontColor = CL_BLUE;		/* ��ɫ */	
	}

	LCD_DispStr(10, 224, buf, &font);

	/* ��ʾͨ��2��Ϣ */
	font.FrontColor = CL_RED; /* CH2 ��ɫ */
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
		font.BackColor = CL_GREEN;		/* ��ɫ */
		font.FrontColor = CL_MAGENTA;	/* ��ɫ */			
	}
	else
	{
		font.BackColor = CL_GREEN;		/* ��ɫ */
		font.FrontColor = CL_BLUE;		/* ��ɫ */			
	}
	LCD_DispStr(120, 224, buf, &font);

	/* ��ʾʱ�� */
	font.FrontColor = CL_WHITE;		/* ��ɫ */		
	font.BackColor = CL_BLUE;		/* ��ɫ */
		
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


	/* ��ʾ����ģʽ */
	font.FrontColor = CL_WHITE;		/* ���� */		
	font.BackColor = CL_BLUE;		/* ���� */
	
	if (g_DSO->AdjustMode == 1)
	{
		LCD_DispStr(245, 2, "����λ��", &font);
	}
	else
	{
		LCD_DispStr(245, 2, "���ڷ���", &font);
	}

	sprintf(buf, "����Ƶ��:%7dHz",	g_DSO->SampleFreq);
	LCD_DispStr(75, 2, buf, &font);
}

/*
*********************************************************************************************************
*	�� �� ��: DispDSO
*	����˵��: DispDSO
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispDSO(void)
{
	DispButton();

	DispFrame();	/* ���ƿ̶ȿ� */
	DispChInfo();	/* ��ʾͨ����Ϣ(���ȣ�ʱ�䵵λ) */

	DispCh1Wave();	/* ��ʾ����1 */	
	DispCh2Wave();	/* ��ʾ����2 */	
}

/*
*********************************************************************************************************
*	�� �� ��: InitDsoParam
*	����˵��: ��ʼ��ȫ�ֲ�������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitDsoParam(void)
{
	g_DSO->Ch1Attenuation = 23;	/* ����ȱʡ˥��ϵ�� */
	g_DSO->Ch2Attenuation = 23;	/* ����ȱʡ˥��ϵ�� */
	g_DSO->Ch1VScale = 1000;	/* ȱʡ��1V */
	g_DSO->Ch2VScale = 1000;	/* ȱʡ��1V */

	g_DSO->Ch1VOffset = 70; 	/* ͨ��1 GND��λ�� */
	g_DSO->Ch2VOffset = 170; /* ͨ��2 GND��λ�� */

	g_DSO->ActiveCH = 1;		/* ȱʡ��CH1 */
	g_DSO->AdjustMode = 1;	/* ȱʡ�ǵ��ڴ�ֱƫ�ƣ� �����л���2���ڷ��� */

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
*	������: IncSampleFreq
*	��  ��:	��
*	��  ��:	��
*	��  �ܣ����Ӳ���Ƶ�ʣ��� 1-2-5 
*	
*	ʱ����ֶȣ�ÿ1������ʱ��)
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
*	�� �� ��: IncSampleFreq
*	����˵��: ���Ӳ���Ƶ�ʣ��� 1-2-5 
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
	ʱ����ֶȣ�ÿ1������ʱ��)
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

	SetSampRate(g_DSO->SampleFreq);	/* �ı����Ƶ�� */
}

/*
*********************************************************************************************************
*	�� �� ��: DecSampleFreq
*	����˵��: ���Ͳ���Ƶ�ʣ��� 1-2-5 
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DecSampleFreq(void)
{
	/*	ʱ����ֶȣ�ÿ1������ʱ��)
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

	SetSampRate(g_DSO->SampleFreq);	/* �ı����Ƶ�� */
}

/*
*********************************************************************************************************
*	�� �� ��: AdjustAtt
*	����˵��: ���ڵ�ѹ˥����λ���� 1-2-5 
*	��    ��: ch   : ͨ���ţ�1��2
*			  mode : 0 ���ͣ� 1����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AdjustAtt(uint8_t ch, uint8_t mode)
{

	if (ch == 1)
	{
		if (mode == 0) 	/* ���� */
		{
			if (g_DSO->Ch1AttId > 0)
			{
				g_DSO->Ch1AttId--;
			}
		}
		else		/* ���� */
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
		if (mode == 0) 	/* ���� */
		{
			if (g_DSO->Ch2AttId > 0)
			{
				g_DSO->Ch2AttId--;
			}
		}
		else		/* ���� */
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
*	�� �� ��: AdjustAtt
*	����˵��: ��ͣADC����
*	��    ��: ��
*	�� �� ֵ: ��
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
*	�� �� ��: StartADC
*	����˵��: ����ADC����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void StartADC(void)
{
#if 1
	DMA_InitTypeDef DMA_InitStructure;

	DMA_DeInit(DMA1_Channel1);		/* ��λDMA1�Ĵ�����ȱʡ״̬ */
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	/* ѡ��ADC1�����ݼĴ�����ΪԴ */
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_DSO->Ch1Buf;	/* Ŀ���ַ */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* ����DMA���䷽������(ADC)��ΪԴ */
	DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* ���û�������С */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* �����ַ������ */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* �洢����ַ��Ҫ���� */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* ѡ�����贫�䵥λ��16bit */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* ѡ���ڴ洫�䵥λ��16bit */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* ����ѭ��ģʽ   */
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	/* ѡ��DMA���ȼ� */
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA�������ͣ������ڴ浽�ڴ� */
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	DMA_DeInit(DMA2_Channel5);		/* ��λDMA2�Ĵ�����ȱʡ״̬ */
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC3_DR_Address;	/* ѡ��ADC1�����ݼĴ�����ΪԴ */
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_DSO->Ch2Buf;	/* Ŀ���ַ */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	/* ����DMA���䷽������(ADC)��ΪԴ */
	DMA_InitStructure.DMA_BufferSize = SAMPLE_COUNT;	/* ���û�������С */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	/* �����ַ������ */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	/* �洢����ַ��Ҫ���� */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	/* ѡ�����贫�䵥λ��16bit */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	/* ѡ���ڴ洫�䵥λ��16bit */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	/* ����ѭ��ģʽ   */
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;	/* ѡ��DMA���ȼ� */
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	/* DMA�������ͣ������ڴ浽�ڴ� */
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
*	�� �� ��: DsoRun
*	����˵��: DSO����״̬
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DsoRun(uint8_t *pMainStatus)
{
	uint8_t KeyCode;
	uint8_t fRefresh = 1;	/* LCDˢ�±�־ */
	FormDSO_T form;	
	uint8_t ucTouch;
	int16_t tpX, tpY;	

	InitDsoParam();	/* ��ʼ��ʾ�������� */

	InitDSO();		/* ����ʾ�����õ������裺ADC,TIM,DMA�� */

	LCD_ClrScr(CL_BLUE);  			/* ������������ɫ */
	
	FormDSO = &form;
	
	/* ����һ����ť */
	{
		/* ��ť���� */
		FormDSO->FontBtn.FontCode = FC_ST_16;
		FormDSO->FontBtn.BackColor = CL_MASK;		/* ͸������ */
		FormDSO->FontBtn.FrontColor = CL_BLACK;
		FormDSO->FontBtn.Space = 0;
		
		/* ��ť */
		FormDSO->BtnRet.Left = BTN_RET_X;
		FormDSO->BtnRet.Top = BTN_RET_Y;
		FormDSO->BtnRet.Height = BTN_RET_H;
		FormDSO->BtnRet.Width = BTN_RET_W;
		FormDSO->BtnRet.pCaption = BTN_RET_T;
		FormDSO->BtnRet.Font = &FormDSO->FontBtn;
		FormDSO->BtnRet.Focus = 0;
		
		LCD_DrawButton(&FormDSO->BtnRet);
	}

	AD9833_SetWaveFreq(100000);		/* Ƶ�� = 10KHz, ��λ��0.1Hz */
	
	AD9833_SetWaveFreq(100000);		/* Ƶ�� = 10KHz, ��λ��0.1Hz */
	AD9833_SelectWave(SINE_WAVE);	/* ���Ҳ� */	
	
	bsp_StartTimer(1, 150);		/* ������ʱ��1��100msˢ��1�� */
	while (*pMainStatus == S_RUN)
	{
		bsp_Idle();

		if (fRefresh)
		{
			fRefresh = 0;

			//StopADC();	/* ��ͣ���� */
			if (g_DSO->HoldEn == 1)
			{
				DispDSO();
			}
			//StartADC();	/* ��ʼ���� */
		}

		if (bsp_CheckTimer(1))
		{
			bsp_StartTimer(1, 150);		/* ������ʱ��1��100msˢ��1�� */

			/* ����״̬��ÿ��100msˢ��1�β��� */
			if (g_DSO->HoldEn == 0)
			{
				StopADC();	/* ��ͣ���� */
				DispDSO();
				StartADC();	/* ��ʼ���� */
			}
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* ��ȡ�����¼� */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* ���ʰ����¼� */
					if (LCD_ButtonTouchDown(&FormDSO->BtnRet, tpX, tpY))
					{
						// *pMainStatus = S_EXIT;
					}
					break;

				case TOUCH_RELEASE:		/* �����ͷ��¼� */
					if (LCD_ButtonTouchRelease(&FormDSO->BtnRet, tpX, tpY))
					{
						*pMainStatus = S_EXIT;
					}
					break;
			}
		}
		
		/* ��ȡ����������0��ʾ�м����� */
		KeyCode = bsp_GetKey();
		if (KeyCode > 0)
		{
			/* �м����� */
			switch (KeyCode)
			{
				case  KEY_DOWN_K1:	/* TAMPER ����ͨ��ѡ��(CH1��CH2) */
					if (g_DSO->ActiveCH == 1)
					{
						g_DSO->ActiveCH = 2;
					}
					else
					{
						g_DSO->ActiveCH = 1;
					}
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case  KEY_DOWN_K2:	/* WAKEUP ��, ����ģʽѡ��(���Ȼ��ߴ�ֱƫ��) */
					/* �˳�,����ȫ������״̬ */
					*pMainStatus = S_HELP;
					break;

				case  KEY_DOWN_K3:	/* USER �� */
					if (g_DSO->HoldEn == 0)
					{
						g_DSO->HoldEn = 1;

						/* ������ͣʱ��ʱ��,Ϊ��ˮƽ��չ�� */
						g_DSO->TimeBaseIdHold = g_DSO->TimeBaseId;	

						StopADC();
					}
					else
					{
						g_DSO->HoldEn = 0;
						StartADC();				
					}
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case JOY_DOWN_L:	/* ҡ��LEFT������ */
					if (g_DSO->HoldEn == 0)
					{
						DecSampleFreq();	/* �ݼ�����Ƶ�� */
						fRefresh = 1;		/* ����ˢ��LCD */
					}
					else
					{
						; /* ����ˮƽ�ƶ��������� */
					}
					break;

				case JOY_DOWN_R:	/* ҡ��RIGHT������ */
					if (g_DSO->HoldEn == 0)
					{
						IncSampleFreq();  	/* ��������Ƶ�� */					
						fRefresh = 1;		/* ����ˢ��LCD */
					}
					else
					{
						; /* ����ˮƽ�ƶ��������� */
					}
					break;

				case  JOY_DOWN_OK:	/* ҡ��OK�� */
					if (g_DSO->AdjustMode == 0)
					{
						g_DSO->AdjustMode = 1;
					}
					else
					{
						g_DSO->AdjustMode = 0;
					}
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case JOY_DOWN_U:	/* ҡ��UP������ */
					if (g_DSO->ActiveCH == 1) 	/* ��ǰ�������CH1 */
					{
						if (g_DSO->AdjustMode == 0)	/* ���ڷ��ȷŴ��� */
						{
							AdjustAtt(1, 1);
						}
						else 	/* ��������ƫ�� */
						{
							g_DSO->Ch1VOffset -= 5;
						}
					}
					else	/* ��ǰ�������CH2 */
					{
						if (g_DSO->AdjustMode == 0)	/* ���ڷ��ȷŴ��� */
						{
							AdjustAtt(2, 1);
						}
						else 	/* ��������ƫ�� */
						{
							g_DSO->Ch2VOffset -= 5;
						}
					}
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				case JOY_DOWN_D:		/* ҡ��DOWN������ */
					if (g_DSO->ActiveCH == 1) 	/* ��ǰ�������CH1 */
					{
						if (g_DSO->AdjustMode == 0)	/* ���ڷ��ȷŴ��� */
						{
							AdjustAtt(1, 0);
						}
						else 	/* ��������ƫ�� */
						{
							g_DSO->Ch1VOffset += 5;
						}
					}
					else	/* ��ǰ�������CH2 */
					{
						if (g_DSO->AdjustMode == 0)	/* ���ڷ��ȷŴ��� */
						{
							AdjustAtt(2, 0);
						}
						else 	/* ��������ƫ�� */
						{
							g_DSO->Ch2VOffset += 5;
						}
					}
					fRefresh = 1;		/* ����ˢ��LCD */
					break;

				default:
					break;
			}
		}
	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

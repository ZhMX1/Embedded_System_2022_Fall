/*
*********************************************************************************************************
*
*	ģ������ : DAC��������
*	�ļ����� : WaveMaker.c
*	��    �� : V1.0
*	˵    �� : ʹ��STM32�ڲ�DAC�������
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2015-06-23  armfly  ��ʽ����
*
*	Copyright (C), 2015-2016, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "WaveMaker.h"

/*
	������PA4���, PA4 ���� DAC_OUT1�� DAC�Ĳο���ѹ = 3.3V
*/

/*  ���Ҳ����ݣ�12bit��1������128����*/
const uint16_t g_SineWave128[] = {
	2047 ,
	2147 ,
	2248 ,
	2347 ,
	2446 ,
	2544 ,
	2641 ,
	2737 ,
	2830 ,
	2922 ,
	3012 ,
	3099 ,
	3184 ,
	3266 ,
	3346 ,
	3422 ,
	3494 ,
	3564 ,
	3629 ,
	3691 ,
	3749 ,
	3803 ,
	3852 ,
	3897 ,
	3938 ,
	3974 ,
	4006 ,
	4033 ,
	4055 ,
	4072 ,
	4084 ,
	4092 ,
	4094 ,
	4092 ,
	4084 ,
	4072 ,
	4055 ,
	4033 ,
	4006 ,
	3974 ,
	3938 ,
	3897 ,
	3852 ,
	3803 ,
	3749 ,
	3691 ,
	3629 ,
	3564 ,
	3494 ,
	3422 ,
	3346 ,
	3266 ,
	3184 ,
	3099 ,
	3012 ,
	2922 ,
	2830 ,
	2737 ,
	2641 ,
	2544 ,
	2446 ,
	2347 ,
	2248 ,
	2147 ,
	2047 ,
	1947 ,
	1846 ,
	1747 ,
	1648 ,
	1550 ,
	1453 ,
	1357 ,
	1264 ,
	1172 ,
	1082 ,
	995  ,
	910  ,
	828  ,
	748  ,
	672  ,
	600  ,
	530  ,
	465  ,
	403  ,
	345  ,
	291  ,
	242  ,
	197  ,
	156  ,
	120  ,
	88   ,
	61   ,
	39   ,
	22   ,
	10   ,
	2    ,
	0    ,
	2    ,
	10   ,
	22   ,
	39   ,
	61   ,
	88   ,
	120  ,
	156  ,
	197  ,
	242  ,
	291  ,
	345  ,
	403  ,
	465  ,
	530  ,
	600  ,
	672  ,
	748  ,
	828  ,
	910  ,
	995  ,
	1082 ,
	1172 ,
	1264 ,
	1357 ,
	1453 ,
	1550 ,
	1648 ,
	1747 ,
	1846 ,
	1947
};

/* ���Ҳ� */
const uint16_t g_SineWave32[32] = {
                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909,
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};

/* ���� */
const uint16_t g_SineWave32_FangBo[32] = {
                      0, 0, 0, 0, 0, 0, 0, 0,  
					  0, 0, 0, 0, 0, 0, 0, 0, 
                      4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
					  4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095
};

uint16_t g_Wave[128];

/*
*********************************************************************************************************
*	�� �� ��: InitWaveMaker
*	����˵��: ����PA4 ΪDAC_OUT1, ����DMA2, ��䲨�����ݡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void InitWaveMaker(void)
{
	{
		uint32_t i;

		for (i = 0; i < 32; i++)
		{
			/* �������Ҳ����� */
			g_Wave[i] = (g_SineWave32[i] * 8) / 10;
		}

	}

   	/* ����DMA, GPIO, DAC, TIM6 ʱ�� */
  	{
		/* DAC Periph clock enable */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
		
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
		
	  	/* GPIOA Periph clock enable */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		
		/* TIM6 Periph clock enable */
		//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	}
	
	/* ����GPIO.  */
	{
		GPIO_InitTypeDef GPIO_InitStructure;

	//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

		/* ����PA4ΪDAC_OUT1 */
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}

	/* TIM6 Configuration */
	#if 1 /* ��TIM6, IAR�²��������� */
	{
	//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

		TIM_PrescalerConfig(TIM6, 0, TIM_PSCReloadMode_Update);

		/* ����DAC����Ĳ���Ƶ��
		  x = 72000000 / Ƶ��
		*/
		//TIM_SetAutoreload(TIM6, 562);	   /* 562 ���1KHz�����Ҳ� */
		//TIM_SetAutoreload(TIM6, 22);	   /* 22 ���100KHz�����Ҳ� */
		TIM_SetAutoreload(TIM6, 220);	   /* 22 ���10KHz�����Ҳ� */

		/* TIM6 TRGO selection */
		TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

	}
	#else
	{
		TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	  
		/* TIM2 Configuration */
		/* Time base configuration */
		TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
		TIM_TimeBaseStructure.TIM_Period = 0x19;          
		TIM_TimeBaseStructure.TIM_Prescaler = 0x0;       
		TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
		
		/* TIM2 TRGO selection */
		TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);	  
	}
	#endif

	/* DAC channel1 Configuration */
	{
		DAC_InitTypeDef DAC_InitStructure;

//		DAC_StructInit(&DAC_InitStructure);
//		RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

		DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
		//DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;
		DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
		DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
		DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
		DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	}

	/* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
	 automatically connected to the DAC converter. */
	DAC_Cmd(DAC_Channel_1, ENABLE);
	
	/* DMA2 channel3 configuration */
	{
		DMA_InitTypeDef DMA_InitStructure;

//		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

		DMA_DeInit(DMA2_Channel3);
		DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1_Address;
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_Wave;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_InitStructure.DMA_BufferSize = 32;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(DMA2_Channel3, &DMA_InitStructure);

		/* Enable DMA2 Channel3 */
		DMA_Cmd(DMA2_Channel3, ENABLE);
	}

	/* Enable DMA for DAC Channel1 */
	DAC_DMACmd(DAC_Channel_1, ENABLE);

	/* TIM6 enable counter */
	TIM_Cmd(TIM6, ENABLE);
	//TIM_Cmd(TIM2, ENABLE);

	//ENABLE_INT();	/* ʹ��ȫ���ж� */	
}


/*
*********************************************************************************************************
*	�� �� ��: SetupWaveType
*	����˵��: ���ò��ε����͡�WT_SIN �� WT_RECT
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SetupWaveType(uint8_t _type)
{
	/*
		���㲨�η��ȡ�
		DAC�Ĵ������ֵ 4096 ��Ӧ3.3V

		�����������2V�ķ��ȣ�ϵ�� = 0.606
		4096 * 0.606 = 2482
	*/
	TIM_Cmd(TIM6, DISABLE);
	switch (_type)
	{
		case WT_SIN:	/* ���Ҳ� */
			{
				uint32_t i;

				for (i = 0; i < 128; i++)
				{
					/* �������Ҳ����� */
					g_Wave[i] = (g_SineWave32[i] * 606) / 1000;
				}

			}
			break;

		case WT_RECT:	/* ���� */
			{
				uint32_t i;

				for (i = 0; i < 64; i++)
				{
					g_Wave[i] = 0;
				}

				for (i = 0; i < 64; i++)
				{
					g_Wave[i] = 2482;
				}
			}
			break;
	}


    TIM_Cmd(TIM6, ENABLE);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

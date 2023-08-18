/*
*********************************************************************************************************
*	                                  
*	ģ������ : CAN����ͨ�ų���
*	�ļ����� : can_network.c
*	��    �� : V1.0
*	˵    �� : ʵ�ֶ��CAN�ڵ㣨�ڵ�ĳ�����ͬ��֮���ͨ�š�
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2015-08-09 armfly  ST�̼���V3.5.0�汾��
*
*	Copyright (C), 2015-2016, ���������� www.armfly.com
*
*********************************************************************************************************
*/

/*
	Ӧ�ò�Э��:���Զ����Э�飩	
	01  01 01 --- ����LEDָʾ�Ƶ����� ��1���ֽ���������룬��2���ֽڱ�ʾ00��ʾ�رգ�01��ʾ������
			    ��3���ֽڱ�ʾָʾ�����(1-4)
	02  00    --- ���Ʒ���������1���ֽڱ�ʾ������룬��2���ֽ� 00 ��ʾ�رգ�01��ʾ�������У� 02��ʾ����1��
*/

#include "bsp.h"
#include "can_network.h"

/* ����ȫ�ֱ��� */
CanTxMsg g_tCanTxMsg;	/* ���ڷ��� */
CanRxMsg g_tCanRxMsg;	/* ���ڽ��� */
uint8_t g_ucLedNo = 0;	/* ������LED����ţ�0-3 */

/* �������ļ��ڵ��õĺ������� */
static void can_NVIC_Config(void);

/*
*********************************************************************************************************
*	�� �� ��: can_SendPacket
*	����˵��: ����һ������
*	��    �Σ�_DataBuf ���ݻ�����
*			  _Len ���ݳ���, 0-8�ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can_SendPacket(uint8_t *_DataBuf, uint8_t _Len)
{		
	/* ��䷢�Ͳ����� Ҳ������ÿ�η��͵�ʱ���� */
	g_tCanTxMsg.StdId = 0x321;
	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_STD;
	g_tCanTxMsg.DLC = _Len;
						
    memcpy(g_tCanTxMsg.Data, _DataBuf, _Len);
    CAN_Transmit(CAN1, &g_tCanTxMsg);	
}

/*
*********************************************************************************************************
*	�� �� ��: can_LedOn
*	����˵��: ����Զ���豸����LED
*	��    �Σ�_addr �豸��ַ 0 ��ʾ�㲥��
*			  _led_no �� LEDָʾ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can_LedOn(uint8_t _addr, uint8_t _led_no)
{		
	/* ��䷢�Ͳ����� Ҳ������ÿ�η��͵�ʱ���� */
	g_tCanTxMsg.StdId = 0x321;
	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_STD;
	
	g_tCanTxMsg.DLC = 3;
						
	g_tCanTxMsg.Data[0] = 0x01;
	g_tCanTxMsg.Data[1] = 0x01;
	g_tCanTxMsg.Data[2] = _led_no;
    CAN_Transmit(CAN1, &g_tCanTxMsg);	
}

/*
*********************************************************************************************************
*	�� �� ��: can_LedOff
*	����˵��: ����Զ���豸�ر�LED
*	��    �Σ�_addr �豸��ַ 0 ��ʾ�㲥��
*			  _led_no �� LEDָʾ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can_LedOff(uint8_t _addr, uint8_t _led_no)
{		
	/* ��䷢�Ͳ����� Ҳ������ÿ�η��͵�ʱ���� */
	g_tCanTxMsg.StdId = 0x321;
	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_STD;
	
	g_tCanTxMsg.DLC = 3;
						
	g_tCanTxMsg.Data[0] = 0x01;
	g_tCanTxMsg.Data[1] = 0x00;		/* 0x00 ��ʾ�ر� */
	g_tCanTxMsg.Data[2] = _led_no;
    CAN_Transmit(CAN1, &g_tCanTxMsg);	
}


/*
*********************************************************************************************************
*	�� �� ��: can_BeepCtrl
*	����˵��: ����Զ���豸�����͹رշ�����
*	��    �Σ�_addr �豸��ַ 0 ��ʾ�㲥��
*			  _led_no �� LEDָʾ�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can_BeepCtrl(uint8_t _addr, uint8_t _cmd)
{		
	// 02  00    --- ���Ʒ���������1���ֽڱ�ʾ������룬��2���ֽ� 00 ��ʾ�رգ�01��ʾ�������У� 02��ʾ����1��
	
	/* ��䷢�Ͳ����� Ҳ������ÿ�η��͵�ʱ���� */
	g_tCanTxMsg.StdId = 0x321;
	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_STD;
	
	g_tCanTxMsg.DLC = 2;
						
	g_tCanTxMsg.Data[0] = 0x02;
	g_tCanTxMsg.Data[1] = _cmd;		/* ��2���ֽ� 00 ��ʾ�رգ�01��ʾ�������У� 02��ʾ����1�� */
    CAN_Transmit(CAN1, &g_tCanTxMsg);	
}

/*
*********************************************************************************************************
*	�� �� ��: can_Init
*	����˵��: ����CANӲ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;	
		
	/* PB8��PB9��������ΪAFIOģʽ, �л���CAN���� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	/* ʹ��GPIOʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* ����CAN�źŽ�������: RX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	/* GPIO����Ϊ��������ģʽ */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* ����CAN�źŷ�������: TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		/* ����Ϊ����������� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* ����GPIO����ٶ� */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* ԭ����CAN���ź�USB��������ͬ�Ŀ��ߣ�������������ʹ�����ŵ���ӳ�书�ܽ�CAN�����л���PB8��PB9 */
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);	/* ʹ��CAN1����ӳ�� */
	
	/* ʹ��CAN����ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	CAN_DeInit(CAN1);						/* ��λCAN�Ĵ��� */
	CAN_StructInit(&CAN_InitStructure);		/* ���CAN�����ṹ���ԱΪȱʡֵ */
	
	/*
		TTCM = time triggered communication mode
		ABOM = automatic bus-off management 
		AWUM = automatic wake-up mode
		NART = no automatic retransmission
		RFLM = receive FIFO locked mode 
		TXFP = transmit FIFO priority		
	*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			/* ��ֹʱ�䴥��ģʽ��������ʱ���), T  */
	CAN_InitStructure.CAN_ABOM = DISABLE;			/* ��ֹ�Զ����߹رչ��� */
	CAN_InitStructure.CAN_AWUM = DISABLE;			/* ��ֹ�Զ�����ģʽ */
	CAN_InitStructure.CAN_NART = DISABLE;			/* ��ֹ�ٲö�ʧ��������Զ��ش����� */
	CAN_InitStructure.CAN_RFLM = DISABLE;			/* ��ֹ����FIFO����ģʽ */
	CAN_InitStructure.CAN_TXFP = DISABLE;			/* ��ֹ����FIFO���ȼ� */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	/* ����CANΪ��������ģʽ */
	
	/* 
		CAN ������ = RCC_APB1Periph_CAN / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		�����У�����CAN������Ϊ500Kbps		
		CAN ������ = 360000000 / 6 / (1 + 6 + 5) / = 500kHz		
	*/
	
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	CAN_InitStructure.CAN_Prescaler = 6;
	CAN_Init(CAN1, &CAN_InitStructure);
	
	/* ����CAN�˲���0 */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;		/* �˲�����ţ�0-13����14���˲��� */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		/* �˲���ģʽ������ID����ģʽ */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	/* 32λ�˲� */
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;					/* �����ID�ĸ�16bit */
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;					/* �����ID�ĵ�16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;				/* ID����ֵ��16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;				/* ID����ֵ��16bit */
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;		/* �˲�����FIFO 0 */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;				/* ʹ���˲��� */
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	/* �˴���ʱ������ */	

	can_NVIC_Config();		/* ����CAN�ж� */	
}     

/*
*********************************************************************************************************
*	�� �� ��: can_DeInit
*	����˵��: �˳�CANӲ��Ӳ�ʣ��ָ�CPU��ص�GPIOΪȱʡ���ر�CAN�ж�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can_DeInit(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, DISABLE);	/* �ر�CAN1ʱ�� */

	{
		NVIC_InitTypeDef  NVIC_InitStructure;
		
		NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&NVIC_InitStructure);
	
		/* CAN FIFO0 ��Ϣ�����ж�ʹ�� */ 
		CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);	
	}
}

/*
*********************************************************************************************************
*	�� �� ��: can_NVIC_Config
*	����˵��: ����CAN�ж�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/  
static void can_NVIC_Config(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* CAN FIFO0 ��Ϣ�����ж�ʹ�� */ 
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);	
}

/*
*********************************************************************************************************
*	�� �� ��: can_ISR
*	����˵��: CAN�жϷ������. ��������� stm32f10x_it.c�б�����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 
void can_ISR(void)
{
	CAN_Receive(CAN1, CAN_FIFO0, &g_tCanRxMsg);
	if ((g_tCanRxMsg.StdId == 0x321) && (g_tCanRxMsg.IDE == CAN_ID_STD) && (g_tCanRxMsg.DLC > 0))
	{
		bsp_PutMsg(MSG_CAN_RX, 0);		/* ���ð���FIFO������һ���յ�485����֡����Ϣ */
		
		/*
	01  01 01 --- ����LEDָʾ�Ƶ����� ��1���ֽ���������룬��2���ֽڱ�ʾ00��ʾ�رգ�01��ʾ������
			    ��3���ֽڱ�ʾָʾ�����(1-4)
	02  00    --- ���Ʒ���������1���ֽڱ�ʾ������룬��2���ֽ� 00 ��ʾ�رգ�01��ʾ����1�Σ� 02��ʾ����1��
		*/
		switch (g_tCanRxMsg.Data[0])
		{
			case 0x01:
				if (g_tCanRxMsg.Data[1] == 0)
				{
					bsp_LedOff(g_tCanRxMsg.Data[2]);	/* �ر�LED */
				}
				else
				{
					bsp_LedOn(g_tCanRxMsg.Data[2]);	/* ��LED */	
				}
				break;
			
			case 0x02:	/* ���Ʒ����� */
				if (g_tCanRxMsg.Data[1] == 0)
				{
					BEEP_Stop();		/* �رշ����� */
				}
				else
				{
					BEEP_Start(8, 8, g_tCanRxMsg.Data[1]);	/* ����80ms��ͣ80ms�� n�� */	
				}	
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: USB_LP_CAN1_RX0_IRQHandler
*	����˵��: CAN�жϷ������. ��������� stm32f10x_it.c�б�����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/ 
#if 0		// �˺����ŵ� stm32f10x_it.c , USB�жϺ�CAN����
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	can_ISR();
}
#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/



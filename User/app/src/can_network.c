/*
*********************************************************************************************************
*	                                  
*	模块名称 : CAN网络通信程序
*	文件名称 : can_network.c
*	版    本 : V1.0
*	说    明 : 实现多个CAN节点（节点的程序相同）之间的通信。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2015-08-09 armfly  ST固件库V3.5.0版本。
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	应用层协议:（自定义简单协议）	
	01  01 01 --- 控制LED指示灯点亮， 第1个字节是命令代码，第2个字节表示00表示关闭，01表示点亮。
			    第3个字节表示指示灯序号(1-4)
	02  00    --- 控制蜂鸣器，第1个字节表示命令代码，第2个字节 00 表示关闭，01表示持续鸣叫， 02表示鸣叫1次
*/

#include "bsp.h"
#include "can_network.h"

/* 定义全局变量 */
CanTxMsg g_tCanTxMsg;	/* 用于发送 */
CanRxMsg g_tCanRxMsg;	/* 用于接收 */
uint8_t g_ucLedNo = 0;	/* 点亮的LED灯序号，0-3 */

/* 仅允许本文件内调用的函数声明 */
static void can_NVIC_Config(void);

/*
*********************************************************************************************************
*	函 数 名: can_SendPacket
*	功能说明: 发送一包数据
*	形    参：_DataBuf 数据缓冲区
*			  _Len 数据长度, 0-8字节
*	返 回 值: 无
*********************************************************************************************************
*/
void can_SendPacket(uint8_t *_DataBuf, uint8_t _Len)
{		
	/* 填充发送参数， 也可以再每次发送的时候填 */
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
*	函 数 名: can_LedOn
*	功能说明: 控制远程设备点亮LED
*	形    参：_addr 设备地址 0 表示广播的
*			  _led_no ： LED指示灯序号
*	返 回 值: 无
*********************************************************************************************************
*/
void can_LedOn(uint8_t _addr, uint8_t _led_no)
{		
	/* 填充发送参数， 也可以再每次发送的时候填 */
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
*	函 数 名: can_LedOff
*	功能说明: 控制远程设备关闭LED
*	形    参：_addr 设备地址 0 表示广播的
*			  _led_no ： LED指示灯序号
*	返 回 值: 无
*********************************************************************************************************
*/
void can_LedOff(uint8_t _addr, uint8_t _led_no)
{		
	/* 填充发送参数， 也可以再每次发送的时候填 */
	g_tCanTxMsg.StdId = 0x321;
	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_STD;
	
	g_tCanTxMsg.DLC = 3;
						
	g_tCanTxMsg.Data[0] = 0x01;
	g_tCanTxMsg.Data[1] = 0x00;		/* 0x00 表示关闭 */
	g_tCanTxMsg.Data[2] = _led_no;
    CAN_Transmit(CAN1, &g_tCanTxMsg);	
}


/*
*********************************************************************************************************
*	函 数 名: can_BeepCtrl
*	功能说明: 控制远程设备启动和关闭蜂鸣器
*	形    参：_addr 设备地址 0 表示广播的
*			  _led_no ： LED指示灯序号
*	返 回 值: 无
*********************************************************************************************************
*/
void can_BeepCtrl(uint8_t _addr, uint8_t _cmd)
{		
	// 02  00    --- 控制蜂鸣器，第1个字节表示命令代码，第2个字节 00 表示关闭，01表示持续鸣叫， 02表示鸣叫1次
	
	/* 填充发送参数， 也可以再每次发送的时候填 */
	g_tCanTxMsg.StdId = 0x321;
	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_STD;
	
	g_tCanTxMsg.DLC = 2;
						
	g_tCanTxMsg.Data[0] = 0x02;
	g_tCanTxMsg.Data[1] = _cmd;		/* 第2个字节 00 表示关闭，01表示持续鸣叫， 02表示鸣叫1次 */
    CAN_Transmit(CAN1, &g_tCanTxMsg);	
}

/*
*********************************************************************************************************
*	函 数 名: can_Init
*	功能说明: 配置CAN硬件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void can_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;	
		
	/* PB8，PB9口线设置为AFIO模式, 切换到CAN功能 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	/* 使能GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* 配置CAN信号接收引脚: RX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	/* GPIO配置为上拉输入模式 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* 配置CAN信号发送引脚: TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		/* 配置为复用推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* 设置GPIO最大速度 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* 原生的CAN引脚和USB引脚是相同的口线，安富莱开发板使用引脚的重映射功能将CAN引脚切换到PB8，PB9 */
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);	/* 使能CAN1的重映射 */
	
	/* 使能CAN外设时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	CAN_DeInit(CAN1);						/* 复位CAN寄存器 */
	CAN_StructInit(&CAN_InitStructure);		/* 填充CAN参数结构体成员为缺省值 */
	
	/*
		TTCM = time triggered communication mode
		ABOM = automatic bus-off management 
		AWUM = automatic wake-up mode
		NART = no automatic retransmission
		RFLM = receive FIFO locked mode 
		TXFP = transmit FIFO priority		
	*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			/* 禁止时间触发模式（不生成时间戳), T  */
	CAN_InitStructure.CAN_ABOM = DISABLE;			/* 禁止自动总线关闭管理 */
	CAN_InitStructure.CAN_AWUM = DISABLE;			/* 禁止自动唤醒模式 */
	CAN_InitStructure.CAN_NART = DISABLE;			/* 禁止仲裁丢失或出错后的自动重传功能 */
	CAN_InitStructure.CAN_RFLM = DISABLE;			/* 禁止接收FIFO加锁模式 */
	CAN_InitStructure.CAN_TXFP = DISABLE;			/* 禁止传输FIFO优先级 */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	/* 设置CAN为正常工作模式 */
	
	/* 
		CAN 波特率 = RCC_APB1Periph_CAN / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		本例中，设置CAN波特率为500Kbps		
		CAN 波特率 = 360000000 / 6 / (1 + 6 + 5) / = 500kHz		
	*/
	
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	CAN_InitStructure.CAN_Prescaler = 6;
	CAN_Init(CAN1, &CAN_InitStructure);
	
	/* 设置CAN滤波器0 */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;		/* 滤波器序号，0-13，共14个滤波器 */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		/* 滤波器模式，设置ID掩码模式 */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	/* 32位滤波 */
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;					/* 掩码后ID的高16bit */
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;					/* 掩码后ID的低16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;				/* ID掩码值高16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;				/* ID掩码值低16bit */
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;		/* 滤波器绑定FIFO 0 */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;				/* 使能滤波器 */
	CAN_FilterInit(&CAN_FilterInitStructure);
	
	/* 此处暂时不发送 */	

	can_NVIC_Config();		/* 配置CAN中断 */	
}     

/*
*********************************************************************************************************
*	函 数 名: can_DeInit
*	功能说明: 退出CAN硬件硬质，恢复CPU相关的GPIO为缺省；关闭CAN中断
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void can_DeInit(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, DISABLE);	/* 关闭CAN1时钟 */

	{
		NVIC_InitTypeDef  NVIC_InitStructure;
		
		NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&NVIC_InitStructure);
	
		/* CAN FIFO0 消息接收中断使能 */ 
		CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);	
	}
}

/*
*********************************************************************************************************
*	函 数 名: can_NVIC_Config
*	功能说明: 配置CAN中断
*	形    参：无
*	返 回 值: 无
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
	
	/* CAN FIFO0 消息接收中断使能 */ 
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);	
}

/*
*********************************************************************************************************
*	函 数 名: can_ISR
*	功能说明: CAN中断服务程序. 这个函数在 stm32f10x_it.c中被调用
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/ 
void can_ISR(void)
{
	CAN_Receive(CAN1, CAN_FIFO0, &g_tCanRxMsg);
	if ((g_tCanRxMsg.StdId == 0x321) && (g_tCanRxMsg.IDE == CAN_ID_STD) && (g_tCanRxMsg.DLC > 0))
	{
		bsp_PutMsg(MSG_CAN_RX, 0);		/* 借用按键FIFO，发送一个收到485数据帧的消息 */
		
		/*
	01  01 01 --- 控制LED指示灯点亮， 第1个字节是命令代码，第2个字节表示00表示关闭，01表示点亮。
			    第3个字节表示指示灯序号(1-4)
	02  00    --- 控制蜂鸣器，第1个字节表示命令代码，第2个字节 00 表示关闭，01表示鸣叫1次， 02表示鸣叫1次
		*/
		switch (g_tCanRxMsg.Data[0])
		{
			case 0x01:
				if (g_tCanRxMsg.Data[1] == 0)
				{
					bsp_LedOff(g_tCanRxMsg.Data[2]);	/* 关闭LED */
				}
				else
				{
					bsp_LedOn(g_tCanRxMsg.Data[2]);	/* 打开LED */	
				}
				break;
			
			case 0x02:	/* 控制蜂鸣器 */
				if (g_tCanRxMsg.Data[1] == 0)
				{
					BEEP_Stop();		/* 关闭蜂鸣器 */
				}
				else
				{
					BEEP_Start(8, 8, g_tCanRxMsg.Data[1]);	/* 鸣叫80ms，停80ms， n次 */	
				}	
				break;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: USB_LP_CAN1_RX0_IRQHandler
*	功能说明: CAN中断服务程序. 这个函数在 stm32f10x_it.c中被调用
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/ 
#if 0		// 此函数放到 stm32f10x_it.c , USB中断和CAN共享
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	can_ISR();
}
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/



/*
*********************************************************************************************************
*
*	模块名称 : 录音演示程序。
*	文件名称 : audio_rec.c
*	版    本 : V1.1
*	说    明 : 演示I2S录音和回放功能。通过串口超级终端作为操作界面。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2013-02-01 armfly  首发
*		V1.1	2014-11-04 armfly  去掉LED1指示灯的控制。因为这个GPIO将用于3.5寸触摸屏。
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	安富莱开发板配置的I2S音频CODEC芯片为WM8978。

	操作提示：
		[TAMPER]键     = 选择音频格式
		[WAKEUP]键     = 开始录音
		[USER]键       = 开始回放
		摇杆上/下键    = 调节音量
		摇杆左/右键    = 调节MIC增益
		摇杆OK键       = 终止录放

	录音时数据保存在安富莱STM32开发板外扩的2MB SRAM中，缺省使用8K采样率，16bit格式。1MB可以保存64秒录音。
	放音按相同的格式，从外部SRAM中读取数据。

	I2S_StartRecord() 和 I2S_StartPlay()函数的形参值域
    接口标准支持：
		#define I2S_Standard_Phillips           ((uint16_t)0x0000)
		#define I2S_Standard_MSB                ((uint16_t)0x0010)
		#define I2S_Standard_LSB                ((uint16_t)0x0020)
		#define I2S_Standard_PCMShort           ((uint16_t)0x0030)
		#define I2S_Standard_PCMLong            ((uint16_t)0x00B0)

    字长支持：
    	#define I2S_DataFormat_16b              ((uint16_t)0x0000)
		#define I2S_DataFormat_16bextended      ((uint16_t)0x0001)
		#define I2S_DataFormat_24b              ((uint16_t)0x0003)
		#define I2S_DataFormat_32b              ((uint16_t)0x0005)

	采样频率支持：
		#define I2S_AudioFreq_192k               ((uint32_t)192000)
		#define I2S_AudioFreq_96k                ((uint32_t)96000)
		#define I2S_AudioFreq_48k                ((uint32_t)48000)
		#define I2S_AudioFreq_44k                ((uint32_t)44100)
		#define I2S_AudioFreq_32k                ((uint32_t)32000)
		#define I2S_AudioFreq_22k                ((uint32_t)22050)
		#define I2S_AudioFreq_16k                ((uint32_t)16000)
		#define I2S_AudioFreq_11k                ((uint32_t)11025)
		#define I2S_AudioFreq_8k                 ((uint32_t)8000)
*/

#include "bsp.h"
#include "audio_rec.h"

#define STR_Help1	"摇杆上/下键 = 调节耳机音量"
#define STR_Help2	"摇杆左/右键 = 调节麦克风增益"
#define STR_Help3	"K1键       = 选择音频格式"

/* 返回按钮的坐标(屏幕右下角) */
#define BUTTON_RET_H	32
#define BUTTON_RET_W	60
#define	BUTTON_RET_X	(g_LcdWidth - BUTTON_RET_W - 4)
#define	BUTTON_RET_Y	(g_LcdHeight - BUTTON_RET_H - 4)
#define	BUTTON_RET_TEXT	"返回"

#define BUTTON_REC_H	32
#define BUTTON_REC_W	74
#define	BUTTON_REC_X	5
#define	BUTTON_REC_Y	180
#define	BUTTON_REC_TEXT	"开始录音"

#define BUTTON_PLAY_H		32
#define BUTTON_PLAY_W		74
#define	BUTTON_PLAY_X		(BUTTON_REC_X + BUTTON_REC_W + 10)
#define	BUTTON_PLAY_Y		BUTTON_REC_Y
#define	BUTTON_PLAY_TEXT	"开始放音"

#define BUTTON_STOP_H		32
#define BUTTON_STOP_W		74
#define	BUTTON_STOP_X		(BUTTON_PLAY_X + BUTTON_PLAY_W + 10)
#define	BUTTON_STOP_Y		BUTTON_REC_Y
#define	BUTTON_STOP_TEXT	"停止"


/* 以下是检查框 */
#define CHECK_SPK_X		BUTTON_REC_X
#define CHECK_SPK_Y		(BUTTON_REC_Y + BUTTON_REC_H + 5)
#define	CHECK_SPK_H		CHECK_BOX_H
#define	CHECK_SPK_W		(CHECK_BOX_W + 5 * 16)		/* 决定触摸有效区域 */
#define CHECK_SPK_TEXT	"打开扬声器"


REC_T g_tRec;

/* 音频格式切换列表(可以自定义) */
#define FMT_COUNT	6		/* 音频格式数组元素个数 */
uint32_t g_FmtList[FMT_COUNT][3] =
{
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_8k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_16k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_22k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_44k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_96k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_192k},
};


/* 仅允许本文件内调用的函数声明 */
static void DispStatus(void);

static void StartPlay(void);
static void StartRecord(void);
static void StopRec(void);

/*
*********************************************************************************************************
*	函 数 名: RecorderDemo
*	功能说明: 录音机主程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RecorderDemo(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	uint8_t fRefresh;		/* 刷屏请求标志,1表示需要刷新 */
	uint8_t fQuit = 0;
	FONT_T tFont, tFontBtn, tFontChk;			/* 定义一个字体结构体变量，用于设置字体参数 */

	char buf[128];
	uint16_t x, y;
	uint16_t usLineCap = 18;

	int16_t tpX, tpY;
	BUTTON_T tBtn;
	CHECK_T tCheck;

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

		/* CheckBox 字体 */
		tFontChk.FontCode = FC_ST_16;
		tFontChk.BackColor = CL_MASK;	/* 透明色 */
		tFontChk.FrontColor = CL_YELLOW;
		tFontChk.Space = 0;
	}

	x = 5;
	y = 3;
	LCD_DispStr(x, y, "WM8978录音机", &tFont);			/* 在(8,3)坐标处显示一串汉字 */
	y += usLineCap;

	/* 测试I2C设备 */
	{
		if (i2c_CheckDevice(WM8978_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "WM8978 Ok (0x%02X)", WM8978_SLAVE_ADDRESS);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "WM8978 Err (0x%02X)", WM8978_SLAVE_ADDRESS);

			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;

		tFont.FrontColor = CL_YELLOW;
		LCD_DispStr(x, y, STR_Help1, &tFont);
		y += usLineCap;

		LCD_DispStr(x, y, STR_Help2, &tFont);
		y += usLineCap;

		LCD_DispStr(x, y, STR_Help3, &tFont);
		y += usLineCap;

		tFont.FrontColor = CL_WHITE;
	}

	/* 配置外部SRAM接口已经在 bsp.c 中的 bsp_Init() 函数执行过 */

	/* 检测WM8978芯片，此函数会自动配置CPU的GPIO */
	if (!wm8978_Init())
	{
		bsp_DelayMS(500);
		return;
	}

	/* 显示按钮 */
	{
		tBtn.Font = &tFontBtn;

		tBtn.Left = BUTTON_RET_X;
		tBtn.Top = BUTTON_RET_Y;
		tBtn.Height = BUTTON_RET_H;
		tBtn.Width = BUTTON_RET_W;
		tBtn.Focus = 0;		/* 未选中 */
		tBtn.pCaption = BUTTON_RET_TEXT;
		LCD_DrawButton(&tBtn);

		tBtn.Left = BUTTON_REC_X;
		tBtn.Top = BUTTON_REC_Y;
		tBtn.Height = BUTTON_REC_H;
		tBtn.Width = BUTTON_REC_W;
		tBtn.Focus = 0;		/* 失去焦点 */
		tBtn.pCaption = BUTTON_REC_TEXT;
		LCD_DrawButton(&tBtn);

		tBtn.Left = BUTTON_PLAY_X;
		tBtn.Top = BUTTON_PLAY_Y;
		tBtn.Height = BUTTON_PLAY_H;
		tBtn.Width = BUTTON_PLAY_W;
		tBtn.Focus = 0;		/* 失去焦点 */
		tBtn.pCaption = BUTTON_PLAY_TEXT;
		LCD_DrawButton(&tBtn);

		tBtn.Left = BUTTON_STOP_X;
		tBtn.Top = BUTTON_STOP_Y;
		tBtn.Height = BUTTON_STOP_H;
		tBtn.Width = BUTTON_STOP_W;
		tBtn.Focus = 0;		/* 失去焦点 */
		tBtn.pCaption = BUTTON_STOP_TEXT;
		LCD_DrawButton(&tBtn);

		/* 显示检查框 */
		tCheck.Font = &tFontChk;

		tCheck.Left = CHECK_SPK_X;
		tCheck.Top = CHECK_SPK_Y;
		tCheck.Height = CHECK_SPK_H;
		tCheck.Width = CHECK_SPK_W;
		if (g_tRec.ucSpkOutEn == 1)
		{
			tCheck.Checked = 1;
		}
		else
		{
			tCheck.Checked = 0;
		}
		tCheck.pCaption = CHECK_SPK_TEXT;
		LCD_DrawCheckBox(&tCheck);
	}


	/* 初始化全局变量 */
	g_tRec.ucVolume = 39;		/* 缺省音量 */
	g_tRec.ucMicGain = 34;			/* 缺省PGA增益 */

	I2S_CODEC_Init();			/* 初始化设置I2S */
	fRefresh = 1;

	g_tRec.ucFmtIdx = 1;		/* 缺省音频格式(16Bit, 16KHz) */
	g_tRec.pAudio = (int16_t *)EXT_SRAM_ADDR;	/* 通过 init16_t * 型指针访问外部SRAM */

	/* 清零录音缓冲区. SRAM容量2M字节 */
	{
		int i;

		for (i = 0 ; i < 1 * 1024 * 1024; i++)
		{
			g_tRec.pAudio[i] = 0;
		}
	}

	g_tRec.ucStatus = STA_IDLE;		/* 首先进入空闲状态 */

	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		/* 集中处理显示界面刷新 */
		if (fRefresh == 1)
		{
			fRefresh = 0;
			DispStatus();		/* 显示当前状态，频率，音量等 */
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
					if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
					{
						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 1;	/* 焦点 */
						tBtn.pCaption = BUTTON_RET_TEXT;
						LCD_DrawButton(&tBtn);
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_REC_X, BUTTON_REC_Y, BUTTON_REC_H, BUTTON_REC_W))
					{
						tBtn.Left = BUTTON_REC_X;
						tBtn.Top = BUTTON_REC_Y;
						tBtn.Height = BUTTON_REC_H;
						tBtn.Width = BUTTON_REC_W;
						tBtn.Focus = 1;		/* 焦点 */
						tBtn.pCaption = BUTTON_REC_TEXT;
						LCD_DrawButton(&tBtn);
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_PLAY_X, BUTTON_PLAY_Y, BUTTON_PLAY_H, BUTTON_PLAY_W))
					{
						tBtn.Left = BUTTON_PLAY_X;
						tBtn.Top = BUTTON_PLAY_Y;
						tBtn.Height = BUTTON_PLAY_H;
						tBtn.Width = BUTTON_PLAY_W;
						tBtn.Focus = 1;		/* 焦点 */
						tBtn.pCaption = BUTTON_PLAY_TEXT;
						LCD_DrawButton(&tBtn);
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_STOP_X, BUTTON_STOP_Y, BUTTON_STOP_H, BUTTON_STOP_W))
					{
						tBtn.Left = BUTTON_STOP_X;
						tBtn.Top = BUTTON_STOP_Y;
						tBtn.Height = BUTTON_STOP_H;
						tBtn.Width = BUTTON_STOP_W;
						tBtn.Focus = 1;		/* 焦点 */
						tBtn.pCaption = BUTTON_STOP_TEXT;
						LCD_DrawButton(&tBtn);
					}
					else if (TOUCH_InRect(tpX, tpY, CHECK_SPK_X, CHECK_SPK_Y, CHECK_SPK_H, CHECK_SPK_W))
					{
						if (g_tRec.ucSpkOutEn)
						{
							g_tRec.ucSpkOutEn = 0;
							tCheck.Checked = 0;

							/* 配置WM8978芯片，输入为AUX接口(收音机)，输出为耳机 */
						}
						else
						{
							g_tRec.ucSpkOutEn = 1;
							tCheck.Checked = 1;

							/* 配置WM8978芯片，输入为AUX接口(收音机)，输出为耳机 和 扬声器 */
						}

						tCheck.Left = CHECK_SPK_X;
						tCheck.Top = CHECK_SPK_Y;
						tCheck.Height = CHECK_SPK_H;
						tCheck.Width = CHECK_SPK_W;
						tCheck.pCaption = CHECK_SPK_TEXT;
						LCD_DrawCheckBox(&tCheck);
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
						tBtn.pCaption = BUTTON_RET_TEXT;
						LCD_DrawButton(&tBtn);

						fQuit = 1;	/* 返回 */
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_REC_X, BUTTON_REC_Y, BUTTON_REC_H, BUTTON_REC_W))
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_REC_X;
						tBtn.Top = BUTTON_REC_Y;
						tBtn.Height = BUTTON_REC_H;
						tBtn.Width = BUTTON_REC_W;
						tBtn.Focus = 0;		/* 失去焦点 */
						tBtn.pCaption = BUTTON_REC_TEXT;
						LCD_DrawButton(&tBtn);

						StartRecord();	/* 开始录音 */
						fRefresh = 1;
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_PLAY_X, BUTTON_PLAY_Y, BUTTON_PLAY_H, BUTTON_PLAY_W))
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_PLAY_X;
						tBtn.Top = BUTTON_PLAY_Y;
						tBtn.Height = BUTTON_PLAY_H;
						tBtn.Width = BUTTON_PLAY_W;
						tBtn.Focus = 0;		/* 失去焦点 */
						tBtn.pCaption = BUTTON_PLAY_TEXT;
						LCD_DrawButton(&tBtn);

						StartPlay();	/* 开始放音 */
						fRefresh = 1;
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_STOP_X, BUTTON_STOP_Y, BUTTON_STOP_H, BUTTON_STOP_W))
					{
						tBtn.Left = BUTTON_STOP_X;
						tBtn.Top = BUTTON_STOP_Y;
						tBtn.Height = BUTTON_STOP_H;
						tBtn.Width = BUTTON_STOP_W;
						tBtn.Focus = 0;		/* 失去焦点 */
						tBtn.pCaption = BUTTON_STOP_TEXT;
						LCD_DrawButton(&tBtn);

						StopRec();		/* 停止录音和放音 */
						fRefresh = 1;
					}
					else	/* 按钮失去焦点 */
					{
						tBtn.Font = &tFontBtn;

						tBtn.Focus = 0;		/* 未选中 */

						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.pCaption = BUTTON_RET_TEXT;
						LCD_DrawButton(&tBtn);

						tBtn.Left = BUTTON_REC_X;
						tBtn.Top = BUTTON_REC_Y;
						tBtn.Height = BUTTON_REC_H;
						tBtn.Width = BUTTON_REC_W;
						tBtn.pCaption = BUTTON_REC_TEXT;
						LCD_DrawButton(&tBtn);

						tBtn.Left = BUTTON_PLAY_X;
						tBtn.Top = BUTTON_PLAY_Y;
						tBtn.Height = BUTTON_PLAY_H;
						tBtn.Width = BUTTON_PLAY_W;
						tBtn.pCaption = BUTTON_PLAY_TEXT;
						LCD_DrawButton(&tBtn);

						tBtn.Left = BUTTON_STOP_X;
						tBtn.Top = BUTTON_STOP_Y;
						tBtn.Height = BUTTON_STOP_H;
						tBtn.Width = BUTTON_STOP_W;
						tBtn.pCaption = BUTTON_STOP_TEXT;
						LCD_DrawButton(&tBtn);
					}
					break;
			}
		}

		/* 处理按键事件 */
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:		/* K1键切换音频格式，在下次开始录音和放音时有效 */
					if (++g_tRec.ucFmtIdx >= FMT_COUNT)
					{
						g_tRec.ucFmtIdx = 0;
					}
					fRefresh = 1;
					break;

				case KEY_DOWN_K2:		/* K2键按下，录音 */
					StartRecord();
					fRefresh = 1;
					break;

				case KEY_DOWN_K3:		/* K3键按下，放音 */
					StartPlay();
					fRefresh = 1;
					break;

				case JOY_DOWN_U:		/* 摇杆UP键按下 */
					if (g_tRec.ucVolume <= VOLUME_MAX - VOLUME_STEP)
					{
						g_tRec.ucVolume += VOLUME_STEP;
						wm8978_SetEarVolume(g_tRec.ucVolume);
						wm8978_SetSpkVolume(g_tRec.ucVolume);
						fRefresh = 1;
					}
					break;

				case JOY_DOWN_D:		/* 摇杆DOWN键按下 */
					if (g_tRec.ucVolume >= VOLUME_STEP)
					{
						g_tRec.ucVolume -= VOLUME_STEP;
						wm8978_SetEarVolume(g_tRec.ucVolume);
						wm8978_SetSpkVolume(g_tRec.ucVolume);
						fRefresh = 1;
					}
					break;

				case JOY_DOWN_L:		/* 摇杆LEFT键按下 */
					if (g_tRec.ucMicGain >= GAIN_STEP)
					{
						g_tRec.ucMicGain -= GAIN_STEP;
						wm8978_SetMicGain(g_tRec.ucMicGain);
						fRefresh = 1;
					}
					break;

				case JOY_DOWN_R:	/* 摇杆RIGHT键按下 */
					if (g_tRec.ucMicGain <= GAIN_MAX - GAIN_STEP)
					{
						g_tRec.ucMicGain += GAIN_STEP;
						wm8978_SetMicGain(g_tRec.ucMicGain);
						fRefresh = 1;
					}
					break;

				case JOY_DOWN_OK:		/* 摇杆OK键按下 */
					StopRec();		/* 停止录音和放音 */
					fRefresh = 1;
					break;

				default:
					break;
			}
		}
	}

	StopRec();		/* 停止录音和放音 */
}

/*
*********************************************************************************************************
*	函 数 名: StartPlay
*	功能说明: 配置WM8978和STM32的I2S开始放音。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void StartPlay(void)
{
	/* 如果已经再录音和放音状态，则需要先停止再开启 */
	g_tRec.ucStatus = STA_STOP_I2S;
	bsp_DelayMS(10);	/* 延迟一段时间，等待I2S中断结束 */
	wm8978_Init();		/* 复位WM8978到复位状态 */
	I2S_Stop();			/* 停止I2S录音和放音 */

	g_tRec.ucStatus = STA_PLAYING;		/* 放音状态 */

	g_tRec.uiCursor = 0;		/* 数据指针复位为0，从头开始放音 */

	/* 配置WM8978芯片，输入为DAC，输出为耳机 */
	wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON | SPK_ON);

	/* 调节音量，左右相同音量 */
	wm8978_SetEarVolume(g_tRec.ucVolume);
	wm8978_SetSpkVolume(g_tRec.ucVolume);

	/* 配置WM8978音频接口为飞利浦标准I2S接口，16bit */ 
	wm8978_CfgAudioIF(I2S_Standard_Phillips, 16, I2S_Mode_MasterTx);

	/* 配置STM32的I2S音频接口(比如飞利浦标准I2S接口，16bit， 8K采样率), 开始放音*/
	I2S_StartPlay(g_FmtList[g_tRec.ucFmtIdx][0], g_FmtList[g_tRec.ucFmtIdx][1], g_FmtList[g_tRec.ucFmtIdx][2]);
}

/*
*********************************************************************************************************
*	函 数 名: StartRecord
*	功能说明: 配置WM8978和STM32的I2S开始录音。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void StartRecord(void)
{
	/* 如果已经再录音和放音状态，则需要先停止再开启 */
	g_tRec.ucStatus = STA_STOP_I2S;
	bsp_DelayMS(10);	/* 延迟一段时间，等待I2S中断结束 */
	wm8978_Init();		/* 复位WM8978到复位状态 */
	I2S_Stop();			/* 停止I2S录音和放音 */

	g_tRec.ucStatus = STA_RECORDING;		/* 录音状态 */

	g_tRec.uiCursor = 0;	/* 数据指针复位为0，从头开始录音 */

	/* 配置WM8978芯片，输入为Mic，输出为耳机 */
	//wm8978_CfgAudioPath(MIC_LEFT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
	//wm8978_CfgAudioPath(MIC_RIGHT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
	wm8978_CfgAudioPath(MIC_LEFT_ON | MIC_RIGHT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);

	/* 调节放音音量，左右相同音量 */
	wm8978_SetEarVolume(g_tRec.ucVolume);

	/* 设置MIC通道增益 */
	wm8978_SetMicGain(g_tRec.ucMicGain);

	/* 配置WM8978音频接口为飞利浦标准I2S接口，16bit */
	wm8978_CfgAudioIF(I2S_Standard_Phillips, 16, I2S_Mode_MasterRx);

	/* 配置STM32的I2S音频接口(比如飞利浦标准I2S接口，16bit， 8K采样率), 开始录音 */
	I2S_StartRecord(g_FmtList[g_tRec.ucFmtIdx][0], g_FmtList[g_tRec.ucFmtIdx][1], g_FmtList[g_tRec.ucFmtIdx][2]);
}

/*
*********************************************************************************************************
*	函 数 名: StopRec
*	功能说明: 停止录音和放音
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void StopRec(void)
{
	g_tRec.ucStatus = STA_IDLE;		/* 待机状态 */
	I2S_Stop();		/* 停止I2S录音和放音 */
	wm8978_Init();	/* 复位WM8978到复位状态 */
}

/*
*********************************************************************************************************
*	函 数 名: DispStatus
*	功能说明: 显示当前状态
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispStatus(void)
{
	FONT_T tFont;
	char buf[128];
	uint16_t x, y;

	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
		tFont.FrontColor = CL_WHITE;	/* 字体颜色 */
		tFont.BackColor = CL_BLUE;	/* 文字背景颜色 */
		tFont.Space = 0;				/* 文字间距，单位 = 像素 */
	}

	x = 5;
	y = 100;
	/* 显示当前音频格式 */
	sprintf(buf, "音频格式: Philips,16Bit,%d.%dkHz     ",
		g_FmtList[g_tRec.ucFmtIdx][2]/1000,
		(g_FmtList[g_tRec.ucFmtIdx][2]%1000)/100
		);

	sprintf(&buf[strlen(buf)], "录音时长: %d.%03d秒    ",
		(EXT_SRAM_SIZE / 2) / g_FmtList[g_tRec.ucFmtIdx][2],
		(((EXT_SRAM_SIZE / 2) * 1000) / g_FmtList[g_tRec.ucFmtIdx][2]) % 1000
		);

	LCD_DispStr(x, y, buf, &tFont);
	y += 18;

	sprintf(buf, "麦克风增益 = %d ", g_tRec.ucMicGain);
	sprintf(&buf[strlen(buf)], "耳机音量 = %d         ", g_tRec.ucVolume);
	LCD_DispStr(x, y, buf, &tFont);
	y += 18;

	if (g_tRec.ucStatus == STA_IDLE)
	{
		sprintf(buf, "状态 = 空闲    ");
	}
	else if (g_tRec.ucStatus == STA_RECORDING)
	{
		sprintf(buf,"状态 = 正在录音");
	}
	else if (g_tRec.ucStatus == STA_PLAYING)
	{
		sprintf(buf,"状态 = 正在回放");
	}
	LCD_DispStr(x, y, buf, &tFont);
}

/*
*********************************************************************************************************
*	函 数 名: I2S_CODEC_DataTransfer
*	功能说明: I2S数据传输函数, 被SPI2 (I2S)中断服务程序调用。录音和放音均在此函数处理。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void I2S_CODEC_DataTransfer(void)
{
	uint16_t usData;
	
	/* 只录左声道(安富莱STM32开发板MIC安装在左声道), 放音时左右声道相同 */
	if (g_tRec.ucStatus == STA_RECORDING)	/* 录音状态 */
	{
		if (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
		{
			usData = SPI_I2S_ReceiveData(SPI2);
			#if 1		/* 录双通道 */
				g_tRec.pAudio[g_tRec.uiCursor] = usData;	/* 保存音频数据 */
				if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
				{
					g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
					/* 禁止I2S2 RXNE中断(接收不空)，需要时再打开 */
					SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
					/* 禁止I2S2 TXE中断(发送缓冲区空)，需要时再打开 */
					SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

					bsp_PutKey(JOY_DOWN_OK);	/* 模拟停止键按下 */
				}
			#else	/* 选择一个声道进行记录 */
				if (SPI_I2S_GetFlagStatus(SPI2, I2S_FLAG_CHSIDE) != SET)	/* 判断是不是左声道 */
				{
					g_tRec.pAudio[g_tRec.uiCursor] = usData;	/* 保存音频数据 */
					if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
					{
						g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
						/* 禁止I2S2 RXNE中断(接收不空)，需要时再打开 */
						SPI_I2S_ITConfig(I2S2ext, SPI_I2S_IT_RXNE, DISABLE);
						/* 禁止I2S2 TXE中断(发送缓冲区空)，需要时再打开 */
						SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

						bsp_PutKey(JOY_DOWN_OK);	/* 模拟停止键按下 */
					}
				}
			#endif
		}

		if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET)
		{
			SPI_I2S_SendData(SPI2, 0);
		}
	}
	else if (g_tRec.ucStatus == STA_PLAYING)	/* 放音状态，右声道的值等于左声道 */
	{
		if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET)
		{
			#if 1		/* 双通道放音 */
				usData = g_tRec.pAudio[g_tRec.uiCursor];		/* 读取音频数据 */
				if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
				{
					g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
					/* 禁止I2S2 RXNE中断(接收不空)，需要时再打开 */
					SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
					/* 禁止I2S2 TXE中断(发送缓冲区空)，需要时再打开 */
					SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

					bsp_PutKey(JOY_DOWN_OK);	/* 模拟停止键按下 */
				}
				SPI_I2S_SendData(SPI2, usData);
			#else	/* 可以将其中1个通道的声音复制到另外1个通道(用于左声道录音或右声道录音) */
				usData = g_tRec.pAudio[g_tRec.uiCursor];		/* 读取音频数据 */
				if (SPI_I2S_GetFlagStatus(SPI2, I2S_FLAG_CHSIDE) != SET)	/* 判断是不是左声道 */
				{
					if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
					{
						g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
						/* 禁止I2S2 RXNE中断(接收不空)，需要时再打开 */
						SPI_I2S_ITConfig(I2S2ext, SPI_I2S_IT_RXNE, DISABLE);
						/* 禁止I2S2 TXE中断(发送缓冲区空)，需要时再打开 */
						SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

						bsp_PutKey(JOY_DOWN_OK);	/* 模拟停止键按下 */
					}
				}
				SPI_I2S_SendData(SPI2, usData);
			#endif
		}
	}
	else 	/* 临时状态 STA_STOP_I2S，用于停止中断 */
	{
		/* 禁止I2S2 RXNE中断(接收不空)，需要时再打开 */
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
		/* 禁止I2S2 TXE中断(发送缓冲区空)，需要时再打开 */
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
	}
}

/*
*********************************************************************************************************
*	函 数 名: SPI2_IRQHandler
*	功能说明: I2S数据传输中断服务程序。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void SPI2_IRQHandler(void)
{
	I2S_CODEC_DataTransfer();
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

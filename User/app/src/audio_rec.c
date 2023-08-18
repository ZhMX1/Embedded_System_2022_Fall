/*
*********************************************************************************************************
*
*	ģ������ : ¼����ʾ����
*	�ļ����� : audio_rec.c
*	��    �� : V1.1
*	˵    �� : ��ʾI2S¼���ͻطŹ��ܡ�ͨ�����ڳ����ն���Ϊ�������档
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2013-02-01 armfly  �׷�
*		V1.1	2014-11-04 armfly  ȥ��LED1ָʾ�ƵĿ��ơ���Ϊ���GPIO������3.5�紥������
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

/*
	���������������õ�I2S��ƵCODECоƬΪWM8978��

	������ʾ��
		[TAMPER]��     = ѡ����Ƶ��ʽ
		[WAKEUP]��     = ��ʼ¼��
		[USER]��       = ��ʼ�ط�
		ҡ����/�¼�    = ��������
		ҡ����/�Ҽ�    = ����MIC����
		ҡ��OK��       = ��ֹ¼��

	¼��ʱ���ݱ����ڰ�����STM32������������2MB SRAM�У�ȱʡʹ��8K�����ʣ�16bit��ʽ��1MB���Ա���64��¼����
	��������ͬ�ĸ�ʽ�����ⲿSRAM�ж�ȡ���ݡ�

	I2S_StartRecord() �� I2S_StartPlay()�������β�ֵ��
    �ӿڱ�׼֧�֣�
		#define I2S_Standard_Phillips           ((uint16_t)0x0000)
		#define I2S_Standard_MSB                ((uint16_t)0x0010)
		#define I2S_Standard_LSB                ((uint16_t)0x0020)
		#define I2S_Standard_PCMShort           ((uint16_t)0x0030)
		#define I2S_Standard_PCMLong            ((uint16_t)0x00B0)

    �ֳ�֧�֣�
    	#define I2S_DataFormat_16b              ((uint16_t)0x0000)
		#define I2S_DataFormat_16bextended      ((uint16_t)0x0001)
		#define I2S_DataFormat_24b              ((uint16_t)0x0003)
		#define I2S_DataFormat_32b              ((uint16_t)0x0005)

	����Ƶ��֧�֣�
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

#define STR_Help1	"ҡ����/�¼� = ���ڶ�������"
#define STR_Help2	"ҡ����/�Ҽ� = ������˷�����"
#define STR_Help3	"K1��       = ѡ����Ƶ��ʽ"

/* ���ذ�ť������(��Ļ���½�) */
#define BUTTON_RET_H	32
#define BUTTON_RET_W	60
#define	BUTTON_RET_X	(g_LcdWidth - BUTTON_RET_W - 4)
#define	BUTTON_RET_Y	(g_LcdHeight - BUTTON_RET_H - 4)
#define	BUTTON_RET_TEXT	"����"

#define BUTTON_REC_H	32
#define BUTTON_REC_W	74
#define	BUTTON_REC_X	5
#define	BUTTON_REC_Y	180
#define	BUTTON_REC_TEXT	"��ʼ¼��"

#define BUTTON_PLAY_H		32
#define BUTTON_PLAY_W		74
#define	BUTTON_PLAY_X		(BUTTON_REC_X + BUTTON_REC_W + 10)
#define	BUTTON_PLAY_Y		BUTTON_REC_Y
#define	BUTTON_PLAY_TEXT	"��ʼ����"

#define BUTTON_STOP_H		32
#define BUTTON_STOP_W		74
#define	BUTTON_STOP_X		(BUTTON_PLAY_X + BUTTON_PLAY_W + 10)
#define	BUTTON_STOP_Y		BUTTON_REC_Y
#define	BUTTON_STOP_TEXT	"ֹͣ"


/* �����Ǽ��� */
#define CHECK_SPK_X		BUTTON_REC_X
#define CHECK_SPK_Y		(BUTTON_REC_Y + BUTTON_REC_H + 5)
#define	CHECK_SPK_H		CHECK_BOX_H
#define	CHECK_SPK_W		(CHECK_BOX_W + 5 * 16)		/* ����������Ч���� */
#define CHECK_SPK_TEXT	"��������"


REC_T g_tRec;

/* ��Ƶ��ʽ�л��б�(�����Զ���) */
#define FMT_COUNT	6		/* ��Ƶ��ʽ����Ԫ�ظ��� */
uint32_t g_FmtList[FMT_COUNT][3] =
{
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_8k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_16k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_22k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_44k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_96k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_192k},
};


/* �������ļ��ڵ��õĺ������� */
static void DispStatus(void);

static void StartPlay(void);
static void StartRecord(void);
static void StopRec(void);

/*
*********************************************************************************************************
*	�� �� ��: RecorderDemo
*	����˵��: ¼����������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RecorderDemo(void)
{
	uint8_t ucKeyCode;		/* �������� */
	uint8_t ucTouch;		/* �����¼� */
	uint8_t fRefresh;		/* ˢ�������־,1��ʾ��Ҫˢ�� */
	uint8_t fQuit = 0;
	FONT_T tFont, tFontBtn, tFontChk;			/* ����һ������ṹ���������������������� */

	char buf[128];
	uint16_t x, y;
	uint16_t usLineCap = 18;

	int16_t tpX, tpY;
	BUTTON_T tBtn;
	CHECK_T tCheck;

	LCD_ClrScr(CL_BLUE);  	/* ������������ɫ */

	/* ����������� */
	{
		tFont.FontCode = FC_ST_16;	/* ������� 16���� */
		tFont.FrontColor = CL_WHITE;	/* ������ɫ */
		tFont.BackColor = CL_BLUE;	/* ���ֱ�����ɫ */
		tFont.Space = 0;				/* ���ּ�࣬��λ = ���� */

		/* ��ť���� */
		tFontBtn.FontCode = FC_ST_16;
		tFontBtn.BackColor = CL_MASK;	/* ͸��ɫ */
		tFontBtn.FrontColor = CL_BLACK;
		tFontBtn.Space = 0;

		/* CheckBox ���� */
		tFontChk.FontCode = FC_ST_16;
		tFontChk.BackColor = CL_MASK;	/* ͸��ɫ */
		tFontChk.FrontColor = CL_YELLOW;
		tFontChk.Space = 0;
	}

	x = 5;
	y = 3;
	LCD_DispStr(x, y, "WM8978¼����", &tFont);			/* ��(8,3)���괦��ʾһ������ */
	y += usLineCap;

	/* ����I2C�豸 */
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

	/* �����ⲿSRAM�ӿ��Ѿ��� bsp.c �е� bsp_Init() ����ִ�й� */

	/* ���WM8978оƬ���˺������Զ�����CPU��GPIO */
	if (!wm8978_Init())
	{
		bsp_DelayMS(500);
		return;
	}

	/* ��ʾ��ť */
	{
		tBtn.Font = &tFontBtn;

		tBtn.Left = BUTTON_RET_X;
		tBtn.Top = BUTTON_RET_Y;
		tBtn.Height = BUTTON_RET_H;
		tBtn.Width = BUTTON_RET_W;
		tBtn.Focus = 0;		/* δѡ�� */
		tBtn.pCaption = BUTTON_RET_TEXT;
		LCD_DrawButton(&tBtn);

		tBtn.Left = BUTTON_REC_X;
		tBtn.Top = BUTTON_REC_Y;
		tBtn.Height = BUTTON_REC_H;
		tBtn.Width = BUTTON_REC_W;
		tBtn.Focus = 0;		/* ʧȥ���� */
		tBtn.pCaption = BUTTON_REC_TEXT;
		LCD_DrawButton(&tBtn);

		tBtn.Left = BUTTON_PLAY_X;
		tBtn.Top = BUTTON_PLAY_Y;
		tBtn.Height = BUTTON_PLAY_H;
		tBtn.Width = BUTTON_PLAY_W;
		tBtn.Focus = 0;		/* ʧȥ���� */
		tBtn.pCaption = BUTTON_PLAY_TEXT;
		LCD_DrawButton(&tBtn);

		tBtn.Left = BUTTON_STOP_X;
		tBtn.Top = BUTTON_STOP_Y;
		tBtn.Height = BUTTON_STOP_H;
		tBtn.Width = BUTTON_STOP_W;
		tBtn.Focus = 0;		/* ʧȥ���� */
		tBtn.pCaption = BUTTON_STOP_TEXT;
		LCD_DrawButton(&tBtn);

		/* ��ʾ���� */
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


	/* ��ʼ��ȫ�ֱ��� */
	g_tRec.ucVolume = 39;		/* ȱʡ���� */
	g_tRec.ucMicGain = 34;			/* ȱʡPGA���� */

	I2S_CODEC_Init();			/* ��ʼ������I2S */
	fRefresh = 1;

	g_tRec.ucFmtIdx = 1;		/* ȱʡ��Ƶ��ʽ(16Bit, 16KHz) */
	g_tRec.pAudio = (int16_t *)EXT_SRAM_ADDR;	/* ͨ�� init16_t * ��ָ������ⲿSRAM */

	/* ����¼��������. SRAM����2M�ֽ� */
	{
		int i;

		for (i = 0 ; i < 1 * 1024 * 1024; i++)
		{
			g_tRec.pAudio[i] = 0;
		}
	}

	g_tRec.ucStatus = STA_IDLE;		/* ���Ƚ������״̬ */

	/* ����������ѭ���� */
	while (fQuit == 0)
	{
		bsp_Idle();

		/* ���д�����ʾ����ˢ�� */
		if (fRefresh == 1)
		{
			fRefresh = 0;
			DispStatus();		/* ��ʾ��ǰ״̬��Ƶ�ʣ������� */
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* ��ȡ�����¼� */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* ���ʰ����¼� */
					if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
					{
						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 1;	/* ���� */
						tBtn.pCaption = BUTTON_RET_TEXT;
						LCD_DrawButton(&tBtn);
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_REC_X, BUTTON_REC_Y, BUTTON_REC_H, BUTTON_REC_W))
					{
						tBtn.Left = BUTTON_REC_X;
						tBtn.Top = BUTTON_REC_Y;
						tBtn.Height = BUTTON_REC_H;
						tBtn.Width = BUTTON_REC_W;
						tBtn.Focus = 1;		/* ���� */
						tBtn.pCaption = BUTTON_REC_TEXT;
						LCD_DrawButton(&tBtn);
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_PLAY_X, BUTTON_PLAY_Y, BUTTON_PLAY_H, BUTTON_PLAY_W))
					{
						tBtn.Left = BUTTON_PLAY_X;
						tBtn.Top = BUTTON_PLAY_Y;
						tBtn.Height = BUTTON_PLAY_H;
						tBtn.Width = BUTTON_PLAY_W;
						tBtn.Focus = 1;		/* ���� */
						tBtn.pCaption = BUTTON_PLAY_TEXT;
						LCD_DrawButton(&tBtn);
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_STOP_X, BUTTON_STOP_Y, BUTTON_STOP_H, BUTTON_STOP_W))
					{
						tBtn.Left = BUTTON_STOP_X;
						tBtn.Top = BUTTON_STOP_Y;
						tBtn.Height = BUTTON_STOP_H;
						tBtn.Width = BUTTON_STOP_W;
						tBtn.Focus = 1;		/* ���� */
						tBtn.pCaption = BUTTON_STOP_TEXT;
						LCD_DrawButton(&tBtn);
					}
					else if (TOUCH_InRect(tpX, tpY, CHECK_SPK_X, CHECK_SPK_Y, CHECK_SPK_H, CHECK_SPK_W))
					{
						if (g_tRec.ucSpkOutEn)
						{
							g_tRec.ucSpkOutEn = 0;
							tCheck.Checked = 0;

							/* ����WM8978оƬ������ΪAUX�ӿ�(������)�����Ϊ���� */
						}
						else
						{
							g_tRec.ucSpkOutEn = 1;
							tCheck.Checked = 1;

							/* ����WM8978оƬ������ΪAUX�ӿ�(������)�����Ϊ���� �� ������ */
						}

						tCheck.Left = CHECK_SPK_X;
						tCheck.Top = CHECK_SPK_Y;
						tCheck.Height = CHECK_SPK_H;
						tCheck.Width = CHECK_SPK_W;
						tCheck.pCaption = CHECK_SPK_TEXT;
						LCD_DrawCheckBox(&tCheck);
					}
					break;

				case TOUCH_RELEASE:		/* �����ͷ��¼� */
					if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 1;	/* ���� */
						tBtn.pCaption = BUTTON_RET_TEXT;
						LCD_DrawButton(&tBtn);

						fQuit = 1;	/* ���� */
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_REC_X, BUTTON_REC_Y, BUTTON_REC_H, BUTTON_REC_W))
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_REC_X;
						tBtn.Top = BUTTON_REC_Y;
						tBtn.Height = BUTTON_REC_H;
						tBtn.Width = BUTTON_REC_W;
						tBtn.Focus = 0;		/* ʧȥ���� */
						tBtn.pCaption = BUTTON_REC_TEXT;
						LCD_DrawButton(&tBtn);

						StartRecord();	/* ��ʼ¼�� */
						fRefresh = 1;
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_PLAY_X, BUTTON_PLAY_Y, BUTTON_PLAY_H, BUTTON_PLAY_W))
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_PLAY_X;
						tBtn.Top = BUTTON_PLAY_Y;
						tBtn.Height = BUTTON_PLAY_H;
						tBtn.Width = BUTTON_PLAY_W;
						tBtn.Focus = 0;		/* ʧȥ���� */
						tBtn.pCaption = BUTTON_PLAY_TEXT;
						LCD_DrawButton(&tBtn);

						StartPlay();	/* ��ʼ���� */
						fRefresh = 1;
					}
					else if (TOUCH_InRect(tpX, tpY, BUTTON_STOP_X, BUTTON_STOP_Y, BUTTON_STOP_H, BUTTON_STOP_W))
					{
						tBtn.Left = BUTTON_STOP_X;
						tBtn.Top = BUTTON_STOP_Y;
						tBtn.Height = BUTTON_STOP_H;
						tBtn.Width = BUTTON_STOP_W;
						tBtn.Focus = 0;		/* ʧȥ���� */
						tBtn.pCaption = BUTTON_STOP_TEXT;
						LCD_DrawButton(&tBtn);

						StopRec();		/* ֹͣ¼���ͷ��� */
						fRefresh = 1;
					}
					else	/* ��ťʧȥ���� */
					{
						tBtn.Font = &tFontBtn;

						tBtn.Focus = 0;		/* δѡ�� */

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

		/* �������¼� */
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* �м����� */
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:		/* K1���л���Ƶ��ʽ�����´ο�ʼ¼���ͷ���ʱ��Ч */
					if (++g_tRec.ucFmtIdx >= FMT_COUNT)
					{
						g_tRec.ucFmtIdx = 0;
					}
					fRefresh = 1;
					break;

				case KEY_DOWN_K2:		/* K2�����£�¼�� */
					StartRecord();
					fRefresh = 1;
					break;

				case KEY_DOWN_K3:		/* K3�����£����� */
					StartPlay();
					fRefresh = 1;
					break;

				case JOY_DOWN_U:		/* ҡ��UP������ */
					if (g_tRec.ucVolume <= VOLUME_MAX - VOLUME_STEP)
					{
						g_tRec.ucVolume += VOLUME_STEP;
						wm8978_SetEarVolume(g_tRec.ucVolume);
						wm8978_SetSpkVolume(g_tRec.ucVolume);
						fRefresh = 1;
					}
					break;

				case JOY_DOWN_D:		/* ҡ��DOWN������ */
					if (g_tRec.ucVolume >= VOLUME_STEP)
					{
						g_tRec.ucVolume -= VOLUME_STEP;
						wm8978_SetEarVolume(g_tRec.ucVolume);
						wm8978_SetSpkVolume(g_tRec.ucVolume);
						fRefresh = 1;
					}
					break;

				case JOY_DOWN_L:		/* ҡ��LEFT������ */
					if (g_tRec.ucMicGain >= GAIN_STEP)
					{
						g_tRec.ucMicGain -= GAIN_STEP;
						wm8978_SetMicGain(g_tRec.ucMicGain);
						fRefresh = 1;
					}
					break;

				case JOY_DOWN_R:	/* ҡ��RIGHT������ */
					if (g_tRec.ucMicGain <= GAIN_MAX - GAIN_STEP)
					{
						g_tRec.ucMicGain += GAIN_STEP;
						wm8978_SetMicGain(g_tRec.ucMicGain);
						fRefresh = 1;
					}
					break;

				case JOY_DOWN_OK:		/* ҡ��OK������ */
					StopRec();		/* ֹͣ¼���ͷ��� */
					fRefresh = 1;
					break;

				default:
					break;
			}
		}
	}

	StopRec();		/* ֹͣ¼���ͷ��� */
}

/*
*********************************************************************************************************
*	�� �� ��: StartPlay
*	����˵��: ����WM8978��STM32��I2S��ʼ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void StartPlay(void)
{
	/* ����Ѿ���¼���ͷ���״̬������Ҫ��ֹͣ�ٿ��� */
	g_tRec.ucStatus = STA_STOP_I2S;
	bsp_DelayMS(10);	/* �ӳ�һ��ʱ�䣬�ȴ�I2S�жϽ��� */
	wm8978_Init();		/* ��λWM8978����λ״̬ */
	I2S_Stop();			/* ֹͣI2S¼���ͷ��� */

	g_tRec.ucStatus = STA_PLAYING;		/* ����״̬ */

	g_tRec.uiCursor = 0;		/* ����ָ�븴λΪ0����ͷ��ʼ���� */

	/* ����WM8978оƬ������ΪDAC�����Ϊ���� */
	wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON | SPK_ON);

	/* ����������������ͬ���� */
	wm8978_SetEarVolume(g_tRec.ucVolume);
	wm8978_SetSpkVolume(g_tRec.ucVolume);

	/* ����WM8978��Ƶ�ӿ�Ϊ�����ֱ�׼I2S�ӿڣ�16bit */ 
	wm8978_CfgAudioIF(I2S_Standard_Phillips, 16, I2S_Mode_MasterTx);

	/* ����STM32��I2S��Ƶ�ӿ�(��������ֱ�׼I2S�ӿڣ�16bit�� 8K������), ��ʼ����*/
	I2S_StartPlay(g_FmtList[g_tRec.ucFmtIdx][0], g_FmtList[g_tRec.ucFmtIdx][1], g_FmtList[g_tRec.ucFmtIdx][2]);
}

/*
*********************************************************************************************************
*	�� �� ��: StartRecord
*	����˵��: ����WM8978��STM32��I2S��ʼ¼����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void StartRecord(void)
{
	/* ����Ѿ���¼���ͷ���״̬������Ҫ��ֹͣ�ٿ��� */
	g_tRec.ucStatus = STA_STOP_I2S;
	bsp_DelayMS(10);	/* �ӳ�һ��ʱ�䣬�ȴ�I2S�жϽ��� */
	wm8978_Init();		/* ��λWM8978����λ״̬ */
	I2S_Stop();			/* ֹͣI2S¼���ͷ��� */

	g_tRec.ucStatus = STA_RECORDING;		/* ¼��״̬ */

	g_tRec.uiCursor = 0;	/* ����ָ�븴λΪ0����ͷ��ʼ¼�� */

	/* ����WM8978оƬ������ΪMic�����Ϊ���� */
	//wm8978_CfgAudioPath(MIC_LEFT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
	//wm8978_CfgAudioPath(MIC_RIGHT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
	wm8978_CfgAudioPath(MIC_LEFT_ON | MIC_RIGHT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);

	/* ���ڷ���������������ͬ���� */
	wm8978_SetEarVolume(g_tRec.ucVolume);

	/* ����MICͨ������ */
	wm8978_SetMicGain(g_tRec.ucMicGain);

	/* ����WM8978��Ƶ�ӿ�Ϊ�����ֱ�׼I2S�ӿڣ�16bit */
	wm8978_CfgAudioIF(I2S_Standard_Phillips, 16, I2S_Mode_MasterRx);

	/* ����STM32��I2S��Ƶ�ӿ�(��������ֱ�׼I2S�ӿڣ�16bit�� 8K������), ��ʼ¼�� */
	I2S_StartRecord(g_FmtList[g_tRec.ucFmtIdx][0], g_FmtList[g_tRec.ucFmtIdx][1], g_FmtList[g_tRec.ucFmtIdx][2]);
}

/*
*********************************************************************************************************
*	�� �� ��: StopRec
*	����˵��: ֹͣ¼���ͷ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void StopRec(void)
{
	g_tRec.ucStatus = STA_IDLE;		/* ����״̬ */
	I2S_Stop();		/* ֹͣI2S¼���ͷ��� */
	wm8978_Init();	/* ��λWM8978����λ״̬ */
}

/*
*********************************************************************************************************
*	�� �� ��: DispStatus
*	����˵��: ��ʾ��ǰ״̬
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispStatus(void)
{
	FONT_T tFont;
	char buf[128];
	uint16_t x, y;

	/* ����������� */
	{
		tFont.FontCode = FC_ST_16;	/* ������� 16���� */
		tFont.FrontColor = CL_WHITE;	/* ������ɫ */
		tFont.BackColor = CL_BLUE;	/* ���ֱ�����ɫ */
		tFont.Space = 0;				/* ���ּ�࣬��λ = ���� */
	}

	x = 5;
	y = 100;
	/* ��ʾ��ǰ��Ƶ��ʽ */
	sprintf(buf, "��Ƶ��ʽ: Philips,16Bit,%d.%dkHz     ",
		g_FmtList[g_tRec.ucFmtIdx][2]/1000,
		(g_FmtList[g_tRec.ucFmtIdx][2]%1000)/100
		);

	sprintf(&buf[strlen(buf)], "¼��ʱ��: %d.%03d��    ",
		(EXT_SRAM_SIZE / 2) / g_FmtList[g_tRec.ucFmtIdx][2],
		(((EXT_SRAM_SIZE / 2) * 1000) / g_FmtList[g_tRec.ucFmtIdx][2]) % 1000
		);

	LCD_DispStr(x, y, buf, &tFont);
	y += 18;

	sprintf(buf, "��˷����� = %d ", g_tRec.ucMicGain);
	sprintf(&buf[strlen(buf)], "�������� = %d         ", g_tRec.ucVolume);
	LCD_DispStr(x, y, buf, &tFont);
	y += 18;

	if (g_tRec.ucStatus == STA_IDLE)
	{
		sprintf(buf, "״̬ = ����    ");
	}
	else if (g_tRec.ucStatus == STA_RECORDING)
	{
		sprintf(buf,"״̬ = ����¼��");
	}
	else if (g_tRec.ucStatus == STA_PLAYING)
	{
		sprintf(buf,"״̬ = ���ڻط�");
	}
	LCD_DispStr(x, y, buf, &tFont);
}

/*
*********************************************************************************************************
*	�� �� ��: I2S_CODEC_DataTransfer
*	����˵��: I2S���ݴ��亯��, ��SPI2 (I2S)�жϷ��������á�¼���ͷ������ڴ˺�������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void I2S_CODEC_DataTransfer(void)
{
	uint16_t usData;
	
	/* ֻ¼������(������STM32������MIC��װ��������), ����ʱ����������ͬ */
	if (g_tRec.ucStatus == STA_RECORDING)	/* ¼��״̬ */
	{
		if (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
		{
			usData = SPI_I2S_ReceiveData(SPI2);
			#if 1		/* ¼˫ͨ�� */
				g_tRec.pAudio[g_tRec.uiCursor] = usData;	/* ������Ƶ���� */
				if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
				{
					g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
					/* ��ֹI2S2 RXNE�ж�(���ղ���)����Ҫʱ�ٴ� */
					SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
					/* ��ֹI2S2 TXE�ж�(���ͻ�������)����Ҫʱ�ٴ� */
					SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

					bsp_PutKey(JOY_DOWN_OK);	/* ģ��ֹͣ������ */
				}
			#else	/* ѡ��һ���������м�¼ */
				if (SPI_I2S_GetFlagStatus(SPI2, I2S_FLAG_CHSIDE) != SET)	/* �ж��ǲ��������� */
				{
					g_tRec.pAudio[g_tRec.uiCursor] = usData;	/* ������Ƶ���� */
					if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
					{
						g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
						/* ��ֹI2S2 RXNE�ж�(���ղ���)����Ҫʱ�ٴ� */
						SPI_I2S_ITConfig(I2S2ext, SPI_I2S_IT_RXNE, DISABLE);
						/* ��ֹI2S2 TXE�ж�(���ͻ�������)����Ҫʱ�ٴ� */
						SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

						bsp_PutKey(JOY_DOWN_OK);	/* ģ��ֹͣ������ */
					}
				}
			#endif
		}

		if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET)
		{
			SPI_I2S_SendData(SPI2, 0);
		}
	}
	else if (g_tRec.ucStatus == STA_PLAYING)	/* ����״̬����������ֵ���������� */
	{
		if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET)
		{
			#if 1		/* ˫ͨ������ */
				usData = g_tRec.pAudio[g_tRec.uiCursor];		/* ��ȡ��Ƶ���� */
				if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
				{
					g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
					/* ��ֹI2S2 RXNE�ж�(���ղ���)����Ҫʱ�ٴ� */
					SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
					/* ��ֹI2S2 TXE�ж�(���ͻ�������)����Ҫʱ�ٴ� */
					SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

					bsp_PutKey(JOY_DOWN_OK);	/* ģ��ֹͣ������ */
				}
				SPI_I2S_SendData(SPI2, usData);
			#else	/* ���Խ�����1��ͨ�����������Ƶ�����1��ͨ��(����������¼����������¼��) */
				usData = g_tRec.pAudio[g_tRec.uiCursor];		/* ��ȡ��Ƶ���� */
				if (SPI_I2S_GetFlagStatus(SPI2, I2S_FLAG_CHSIDE) != SET)	/* �ж��ǲ��������� */
				{
					if (++g_tRec.uiCursor >= EXT_SRAM_SIZE / 2)
					{
						g_tRec.uiCursor = EXT_SRAM_SIZE / 2;
						/* ��ֹI2S2 RXNE�ж�(���ղ���)����Ҫʱ�ٴ� */
						SPI_I2S_ITConfig(I2S2ext, SPI_I2S_IT_RXNE, DISABLE);
						/* ��ֹI2S2 TXE�ж�(���ͻ�������)����Ҫʱ�ٴ� */
						SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

						bsp_PutKey(JOY_DOWN_OK);	/* ģ��ֹͣ������ */
					}
				}
				SPI_I2S_SendData(SPI2, usData);
			#endif
		}
	}
	else 	/* ��ʱ״̬ STA_STOP_I2S������ֹͣ�ж� */
	{
		/* ��ֹI2S2 RXNE�ж�(���ղ���)����Ҫʱ�ٴ� */
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
		/* ��ֹI2S2 TXE�ж�(���ͻ�������)����Ҫʱ�ٴ� */
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: SPI2_IRQHandler
*	����˵��: I2S���ݴ����жϷ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void SPI2_IRQHandler(void)
{
	I2S_CODEC_DataTransfer();
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

/*
*********************************************************************************************************
*
*	ģ������ : RA8875����ҵĴ���Flashģ��
*	�ļ����� : ra8875_flash.c
*	��    �� : V1.0
*	˵    �� : ��дRA8875���ϵ��ֿ�оƬ��ͼ��оƬ��RA8875����֧��д����Flash ��ֻ֧��DMA��ʽ�����ݡ�
*			   ͨ�����Ӷ���ĵ��ӿ��ؽ�����Flash�л���STM32�Ϳ��Ը�д���ݡ������֧�ָù��ܵ�RA8875����
*			   ������ RA8875 7����(D017-2)֧������ˢ���ֿ��ͼ��оƬ��
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2013-02-01 armfly  �׷�
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "ra8875_flash.h"
#include "ff.h"			/* FatFS�ļ�ϵͳģ��*/

/* �������ṹ */
typedef struct
{
	FONT_T FontBlack;	/* ��̬������ */
	FONT_T FontBlue;	/* �仯���������� */
	FONT_T FontBtn;		/* ��ť������ */
	FONT_T FontBox;		/* ������������ */

	GROUP_T Box1;
	GROUP_T Box2;

	LABEL_T Label1;	LABEL_T Label2;
	LABEL_T Label3; LABEL_T Label4;
	LABEL_T Label5; LABEL_T Label6;
	LABEL_T Label7; LABEL_T Label8;

	BUTTON_T BtnRet;

	BUTTON_T Btn1;		/* ���ֿ�оƬ */
	BUTTON_T Btn2;		/* д�ֿ�оƬ */
	BUTTON_T Btn3;		/* ��ͼƬоƬ */
	BUTTON_T Btn4;		/* ���ֿ�оƬ */
	BUTTON_T Btn5;		/* �������ߣ����Թ��� */

	BUTTON_T Btn6;		/* ����NORFlash */
	BUTTON_T Btn7;		/* дNORFlash */
}FormW25_T;

/* ���屳��ɫ */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* �������ʹ�С */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	100
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"RA8875�ֿ��ͼ��оƬ"

/* ��1�����ڵİ�ť */
#define BTN1_H	32
#define BTN1_W	120
#define	BTN1_X	(BOX1_X + 10)
#define	BTN1_Y	(BOX1_Y + 20)
#define	BTN1_TEXT	"���ֿ�оƬ"

#define BTN2_H	BTN1_H
#define BTN2_W	BTN1_W
#define	BTN2_X	(BTN1_X +  BTN1_W + 10)
#define	BTN2_Y	BTN1_Y
#define	BTN2_TEXT	"д�ֿ�оƬ"

#define BTN3_H	BTN1_H
#define BTN3_W	BTN1_W
#define	BTN3_X	BTN1_X
#define	BTN3_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN3_TEXT	"��ͼ��оƬ"

#define BTN4_H	BTN1_H
#define BTN4_W	BTN1_W
#define	BTN4_X	(BTN1_X +  BTN1_W + 10)
#define	BTN4_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN4_TEXT	"дͼƬоƬ"

#define BTN5_H	BTN1_H
#define BTN5_W	BTN1_W
#define	BTN5_X	(BTN1_X +  2 * (BTN1_W + 10))
#define	BTN5_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN5_TEXT	"�ر�LCD"


/* ��2���� */
#define BOX2_X	BOX1_X
#define BOX2_Y	(BOX1_Y + BOX1_H + 10)
#define BOX2_H	60
#define BOX2_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX2_TEXT	"NOR FlashоƬ"

#define BTN6_H	32
#define BTN6_W	120
#define	BTN6_X	(BOX2_X + 10)
#define	BTN6_Y	(BOX2_Y + 20)
#define	BTN6_TEXT	"����NOR Flash"

#define BTN7_H	BTN6_H
#define BTN7_W	BTN6_W
#define	BTN7_X	(BTN6_X +  1 * (BTN1_W + 10))
#define	BTN7_Y	BTN6_Y
#define	BTN7_TEXT	"дNOR Flash"

/* ��ť */
/* ���ذ�ť������(��Ļ���½�) */
#define BTN_RET_H	32
#define BTN_RET_W	80
#define	BTN_RET_X	(g_LcdWidth - BTN_RET_W - 8)
#define	BTN_RET_Y	(g_LcdHeight - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"����"


#define LABEL1_X  	(BOX2_X + 6)
#define LABEL1_Y	(BOX2_Y + BOX2_H + 10)
#define LABEL1_TEXT	"��ַ : "

	#define LABEL2_X  	(LABEL1_X + 64)
	#define LABEL2_Y	LABEL1_Y
	#define LABEL2_TEXT	"0"

#define LABEL3_X  	(LABEL1_X)
#define LABEL3_Y	(LABEL1_Y + 20)
#define LABEL3_TEXT	"��ʱ : "

	#define LABEL4_X  	(LABEL3_X + 64)
	#define LABEL4_Y	(LABEL3_Y)
	#define LABEL4_TEXT	"0"

#define LABEL5_X  	(LABEL1_X)
#define LABEL5_Y	(LABEL1_Y + 20 * 2)
#define LABEL5_TEXT	"״̬ : "

	#define LABEL6_X  	(LABEL5_X + 64)
	#define LABEL6_Y	LABEL5_Y
	#define LABEL6_TEXT	"0.0"

#define LABEL7_X  	(LABEL1_X)
#define LABEL7_Y	(LABEL1_Y + 20 * 3)
#define LABEL7_TEXT	"xxxx : "

	#define LABEL8_X  	(LABEL7_X + 64)
	#define LABEL8_Y	LABEL7_Y
	#define LABEL8_TEXT	"0.0"

static void InitFormW25(void);
static void DispFormW25(void);

void ReadRA8875Flash(uint8_t _ucChipType);
void WriteRA8875Flash(uint8_t _ucChipType);
static void EraseNorFlash(void);
static void WriteNorFlash(void);

FormW25_T *FormW25;

/*
*********************************************************************************************************
*	�� �� ��: RA8875FlashMain
*	����˵��: ��дRA8875��ҵĴ���Flash
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875FlashMain(void)
{
	uint8_t ucKeyCode;		/* �������� */
	uint8_t ucTouch;		/* �����¼� */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	FormW25_T form;

	FormW25= &form;

	InitFormW25();

	DispFormW25();

	/* ����������ѭ���� */
	while (fQuit == 0)
	{
		bsp_Idle();

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* ��ȡ�����¼� */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* ���ʰ����¼� */
					LCD_ButtonTouchDown(&FormW25->BtnRet, tpX, tpY);
					if (g_ChipID == IC_8875)
					{
						LCD_ButtonTouchDown(&FormW25->Btn1, tpX, tpY);
						LCD_ButtonTouchDown(&FormW25->Btn2, tpX, tpY);
						LCD_ButtonTouchDown(&FormW25->Btn3, tpX, tpY);
						LCD_ButtonTouchDown(&FormW25->Btn4, tpX, tpY);
						LCD_ButtonTouchDown(&FormW25->Btn5, tpX, tpY);
					}
					LCD_ButtonTouchDown(&FormW25->Btn6, tpX, tpY);
					LCD_ButtonTouchDown(&FormW25->Btn7, tpX, tpY);
					break;

				case TOUCH_RELEASE:		/* �����ͷ��¼� */
					if (LCD_ButtonTouchRelease(&FormW25->BtnRet, tpX, tpY))
					{
						fQuit = 1;	/* ���� */
					}

					if (g_ChipID == IC_8875)
					{
						if (LCD_ButtonTouchRelease(&FormW25->Btn1, tpX, tpY))
						{
							ReadRA8875Flash(FONT_CHIP);	/* ���ֿ�оƬ */
						}
						if (LCD_ButtonTouchRelease(&FormW25->Btn2, tpX, tpY))
						{
							WriteRA8875Flash(FONT_CHIP);	/* д�ֿ�оƬ */
						}
						 if (LCD_ButtonTouchRelease(&FormW25->Btn3, tpX, tpY))
						{
							ReadRA8875Flash(BMP_CHIP);	/* ��ͼ��оƬ */
						}
						if (LCD_ButtonTouchRelease(&FormW25->Btn4, tpX, tpY))
						{
							WriteRA8875Flash(BMP_CHIP);	/* дͼ��оƬ */
						}
						if (LCD_ButtonTouchRelease(&FormW25->Btn5, tpX, tpY))
						{
							RA8875_Sleep();	/* �ر���ʾ���������� */
							while (1);
						}						
					}

					if (LCD_ButtonTouchRelease(&FormW25->Btn6, tpX, tpY))
					{
						EraseNorFlash();		/* ����NORFlash */
					}
					if (LCD_ButtonTouchRelease(&FormW25->Btn7, tpX, tpY))
					{
						WriteNorFlash();		/* дNORFlash */
					}
					
					/* �崥�������������ⳤʱ��ȴ����û���������ť��������дһ�� */
					TOUCH_CelarFIFO();				
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
				case KEY_DOWN_K1:		/* K1�� */
					WriteRA8875Flash(FONT_CHIP);	/* д�ֿ�оƬ */
					break;

				case KEY_DOWN_K2:		/* K2������ */
					WriteRA8875Flash(BMP_CHIP);	/* дͼ��оƬ */
					break;

				case KEY_DOWN_K3:		/* K3������ */
					break;

				case JOY_DOWN_U:		/* ҡ��UP������ */
					break;

				case JOY_DOWN_D:		/* ҡ��DOWN������ */
					break;

				case JOY_DOWN_L:		/* ҡ��LEFT������ */
					break;

				case JOY_DOWN_R:		/* ҡ��RIGHT������ */
					fQuit = 1;	/* ���� */
					break;

				case JOY_DOWN_OK:		/* ҡ��OK������ */
					break;

				default:
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: InitFormW25
*	����˵��: ��ʼ���ؼ�����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitFormW25(void)
{
	/* ������������ */
	FormW25->FontBox.FontCode = FC_ST_16;
	FormW25->FontBox.BackColor = CL_BTN_FACE;	/* �ͱ���ɫ��ͬ */
	FormW25->FontBox.FrontColor = CL_BLACK;
	FormW25->FontBox.Space = 0;

	/* ����1 ���ھ�ֹ��ǩ */
	FormW25->FontBlack.FontCode = FC_ST_16;
	FormW25->FontBlack.BackColor = CL_MASK;		/* ͸��ɫ */
	FormW25->FontBlack.FrontColor = CL_BLACK;
	FormW25->FontBlack.Space = 0;

	/* ����2 ���ڱ仯������ */
	FormW25->FontBlue.FontCode = FC_ST_16;
	FormW25->FontBlue.BackColor = CL_BTN_FACE;
	FormW25->FontBlue.FrontColor = CL_BLUE;
	FormW25->FontBlue.Space = 0;

	/* ��ť���� */
	FormW25->FontBtn.FontCode = FC_ST_16;
	FormW25->FontBtn.BackColor = CL_MASK;		/* ͸������ */
	FormW25->FontBtn.FrontColor = CL_BLACK;
	FormW25->FontBtn.Space = 0;

	/* ����� */
	FormW25->Box1.Left = BOX1_X;
	FormW25->Box1.Top = BOX1_Y;
	FormW25->Box1.Height = BOX1_H;
	FormW25->Box1.Width = BOX1_W;
	FormW25->Box1.pCaption = BOX1_TEXT;
	FormW25->Box1.Font = &FormW25->FontBox;

	FormW25->Box2.Left = BOX2_X;
	FormW25->Box2.Top = BOX2_Y;
	FormW25->Box2.Height = BOX2_H;
	FormW25->Box2.Width = BOX2_W;
	FormW25->Box2.pCaption = BOX2_TEXT;
	FormW25->Box2.Font = &FormW25->FontBox;

	/* ��̬��ǩ */
	FormW25->Label1.Left = LABEL1_X;
	FormW25->Label1.Top = LABEL1_Y;
	FormW25->Label1.MaxLen = 0;
	FormW25->Label1.pCaption = LABEL1_TEXT;
	FormW25->Label1.Font = &FormW25->FontBlack;

	FormW25->Label3.Left = LABEL3_X;
	FormW25->Label3.Top = LABEL3_Y;
	FormW25->Label3.MaxLen = 0;
	FormW25->Label3.pCaption = LABEL3_TEXT;
	FormW25->Label3.Font = &FormW25->FontBlack;

	FormW25->Label5.Left = LABEL5_X;
	FormW25->Label5.Top = LABEL5_Y;
	FormW25->Label5.MaxLen = 0;
	FormW25->Label5.pCaption = LABEL5_TEXT;
	FormW25->Label5.Font = &FormW25->FontBlack;

	FormW25->Label7.Left = LABEL7_X;
	FormW25->Label7.Top = LABEL7_Y;
	FormW25->Label7.MaxLen = 0;
	FormW25->Label7.pCaption = LABEL7_TEXT;
	FormW25->Label7.Font = &FormW25->FontBlack;

	/* ��̬��ǩ */
	FormW25->Label2.Left = LABEL2_X;
	FormW25->Label2.Top = LABEL2_Y;
	FormW25->Label2.MaxLen = 0;
	FormW25->Label2.pCaption = LABEL2_TEXT;
	FormW25->Label2.Font = &FormW25->FontBlue;

	FormW25->Label4.Left = LABEL4_X;
	FormW25->Label4.Top = LABEL4_Y;
	FormW25->Label4.MaxLen = 0;
	FormW25->Label4.pCaption = LABEL4_TEXT;
	FormW25->Label4.Font = &FormW25->FontBlue;

	FormW25->Label6.Left = LABEL6_X;
	FormW25->Label6.Top = LABEL6_Y;
	FormW25->Label6.MaxLen = 0;
	FormW25->Label6.pCaption = LABEL6_TEXT;
	FormW25->Label6.Font = &FormW25->FontBlue;

	FormW25->Label8.Left = LABEL8_X;
	FormW25->Label8.Top = LABEL8_Y;
	FormW25->Label8.MaxLen = 0;
	FormW25->Label8.pCaption = LABEL8_TEXT;
	FormW25->Label8.Font = &FormW25->FontBlue;

	/* ��ť */
	FormW25->BtnRet.Left = BTN_RET_X;
	FormW25->BtnRet.Top = BTN_RET_Y;
	FormW25->BtnRet.Height = BTN_RET_H;
	FormW25->BtnRet.Width = BTN_RET_W;
	FormW25->BtnRet.pCaption = BTN_RET_TEXT;
	FormW25->BtnRet.Font = &FormW25->FontBtn;
	FormW25->BtnRet.Focus = 0;

	FormW25->Btn1.Left = BTN1_X;
	FormW25->Btn1.Top = BTN1_Y;
	FormW25->Btn1.Height = BTN1_H;
	FormW25->Btn1.Width = BTN1_W;
	FormW25->Btn1.pCaption = BTN1_TEXT;
	FormW25->Btn1.Font = &FormW25->FontBtn;
	FormW25->Btn1.Focus = 0;

	FormW25->Btn2.Left = BTN2_X;
	FormW25->Btn2.Top = BTN2_Y;
	FormW25->Btn2.Height = BTN2_H;
	FormW25->Btn2.Width = BTN2_W;
	FormW25->Btn2.pCaption = BTN2_TEXT;
	FormW25->Btn2.Font = &FormW25->FontBtn;
	FormW25->Btn2.Focus = 0;

	FormW25->Btn3.Left = BTN3_X;
	FormW25->Btn3.Top = BTN3_Y;
	FormW25->Btn3.Height = BTN3_H;
	FormW25->Btn3.Width = BTN3_W;
	FormW25->Btn3.pCaption = BTN3_TEXT;
	FormW25->Btn3.Font = &FormW25->FontBtn;
	FormW25->Btn3.Focus = 0;

	FormW25->Btn4.Left = BTN4_X;
	FormW25->Btn4.Top = BTN4_Y;
	FormW25->Btn4.Height = BTN4_H;
	FormW25->Btn4.Width = BTN4_W;
	FormW25->Btn4.pCaption = BTN4_TEXT;
	FormW25->Btn4.Font = &FormW25->FontBtn;
	FormW25->Btn4.Focus = 0;

	FormW25->Btn5.Left = BTN5_X;
	FormW25->Btn5.Top = BTN5_Y;
	FormW25->Btn5.Height = BTN5_H;
	FormW25->Btn5.Width = BTN5_W;
	FormW25->Btn5.pCaption = BTN5_TEXT;
	FormW25->Btn5.Font = &FormW25->FontBtn;
	FormW25->Btn5.Focus = 0;

	FormW25->Btn6.Left = BTN6_X;
	FormW25->Btn6.Top = BTN6_Y;
	FormW25->Btn6.Height = BTN6_H;
	FormW25->Btn6.Width = BTN6_W;
	FormW25->Btn6.pCaption = BTN6_TEXT;
	FormW25->Btn6.Font = &FormW25->FontBtn;
	FormW25->Btn6.Focus = 0;

	FormW25->Btn7.Left = BTN7_X;
	FormW25->Btn7.Top = BTN7_Y;
	FormW25->Btn7.Height = BTN7_H;
	FormW25->Btn7.Width = BTN7_W;
	FormW25->Btn7.pCaption = BTN7_TEXT;
	FormW25->Btn7.Font = &FormW25->FontBtn;
	FormW25->Btn7.Focus = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: DispFormW25
*	����˵��: ��ʾ���еľ�̬�ؼ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispFormW25(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* ����� */
	LCD_DrawGroupBox(&FormW25->Box1);
	LCD_DrawGroupBox(&FormW25->Box2);

	LCD_DrawLabel(&FormW25->Label1);
	LCD_DrawLabel(&FormW25->Label3);
	LCD_DrawLabel(&FormW25->Label5);
//	LCD_DrawLabel(&FormW25->Label7);

	LCD_DrawLabel(&FormW25->Label2);
	LCD_DrawLabel(&FormW25->Label4);
	LCD_DrawLabel(&FormW25->Label6);
//	LCD_DrawLabel(&FormW25->Label8);

	/* ��ť */
	LCD_DrawButton(&FormW25->BtnRet);
	LCD_DrawButton(&FormW25->Btn1);
	LCD_DrawButton(&FormW25->Btn2);
	LCD_DrawButton(&FormW25->Btn3);
	LCD_DrawButton(&FormW25->Btn4);
	LCD_DrawButton(&FormW25->Btn5);
	LCD_DrawButton(&FormW25->Btn6);
	LCD_DrawButton(&FormW25->Btn7);
}

/*
*********************************************************************************************************
*	�� �� ��: ReadRA8875Flash
*	����˵��: ��RA8875 Flash����
*	��    ��: _ucChipType : оƬ���� FONT_CHIP �� BMP_CHIP
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void ReadRA8875Flash(uint8_t _ucChipType)
{
	uint32_t i;
	uint8_t buf[512];
	char cDispBuf[32];
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char path[32];
	uint8_t ucVolume = FS_NAND;
	char volume_str[5];
	int32_t iTime1, iTime2;

	/* ����RA8875����Flash��ص�Ӳ�� */
	bsp_InitRA8875Flash();
	w25_CtrlByMCU();
	w25_SelectChip(_ucChipType);

	/* ������Ҫ�ж�RA8875���Ƿ�֧�ִ���Flash�Ķ�д */
	{
		/* ʶ��Flash�Ƿ�ɶ�д */
		if ((g_tW25.ChipID != W25Q64BV) && (g_tW25.ChipID != W25Q128))
		{
			if ((_ucChipType == FONT_CHIP) && (g_tW25.ChipID == 0))
			{
				/* ��������� */
				w25_ReadBuffer(buf, 0x1FFFF0, 5);

				if (memcmp(buf, "GT20K", 5) == 0)
				{
					/* ��װ�ļ�ͨ�ֿ�оƬ(���Զ�ȡ) */
				}
				else
				{
					FormW25->Label6.pCaption = "��ʾģ��Ӳ����֧�ֶ��ֿ�оƬ";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit2;
				}
			}
			else
			{
				if (_ucChipType == FONT_CHIP)
				{
					FormW25->Label6.pCaption = "��ʾģ��Ӳ����֧�ֶ��ֿ�оƬ";
				}
				else
				{
					FormW25->Label6.pCaption = "��ʾģ��Ӳ����֧�ֶ�ͼ��оƬ";
				}
				LCD_DrawLabel(&FormW25->Label6);

				goto quit2;
			}
		}
	}

	/* �ļ�ϵͳ���� */
	{
		/* �����ļ�ϵͳ */
		sprintf(volume_str, "%d:", ucVolume);
		result = f_mount(&fs, volume_str, 0);			/* Mount a logical drive */
		if (result != FR_OK)
		{
			printf("�����ļ�ϵͳʧ�� (%d)\r\n", result);

			FormW25->Label6.pCaption = "�����ļ�ϵͳʧ��                  ";
			LCD_DrawLabel(&FormW25->Label6);

			goto quit1;
		}

		/* �򿪸��ļ��� */
		sprintf(path, "%d:/RA8875", ucVolume);
		result = f_opendir(&DirInf, path);
		if (result != FR_OK)
		{
			printf("�򿪸�Ŀ¼ʧ�� (%d)\r\n", result);

			FormW25->Label6.pCaption = "�򿪸�Ŀ¼ʧ��             ";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}

		/* ���ļ� */
		if (_ucChipType == FONT_CHIP)
		{
			sprintf(path, "%d:/RA8875/FontRead.bin", ucVolume);

			FormW25->Label6.pCaption = "���ڶ��ֿ�оƬ, ���浽�ļ�FontRead.bin";
			LCD_DrawLabel(&FormW25->Label6);
		}
		else
		{
			sprintf(path, "%d:/RA8875/BmpRead.bin", ucVolume);

			FormW25->Label6.pCaption = "���ڶ�ͼ��оƬ, ���浽�ļ�BmpRead.bin";
			LCD_DrawLabel(&FormW25->Label6);
		}
		result = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);
	}

	iTime1 = bsp_GetRunTime();
	/* ��Label������ʾ���� */
	FormW25->Label2.pCaption = cDispBuf;
	FormW25->Label4.pCaption = cDispBuf;

	for (i = 0; i < g_tW25.TotalSize / 512; i++)
	{
		sprintf(cDispBuf, "0x%06X (%%%02d)  ", i * 512, ((i + 1) * 100) /(g_tW25.TotalSize / 512));
		LCD_DrawLabel(&FormW25->Label2);

		iTime2 = bsp_GetRunTime() - iTime1;
		sprintf(cDispBuf, "%d ��      ", iTime2 / 1000);
		LCD_DrawLabel(&FormW25->Label4);

		w25_ReadBuffer(buf, i * 512, 512);

		/* д256�ֽ����� */
		result = f_write(&file, buf, 512, &bw);
		if (result != FR_OK)
		{
			printf("%s �ļ�д��ʧ��\r\n", path);

			FormW25->Label6.pCaption = "�ļ�д��ʧ��";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}
	}

	FormW25->Label6.pCaption = "���ɹ�                                         ";
	LCD_DrawLabel(&FormW25->Label6);

quit1:
	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	f_mount(NULL, volume_str, 0);

quit2:
	w25_CtrlByRA8875();
}

/*
*********************************************************************************************************
*	�� �� ��: WriteRA8875Flash
*	����˵��: дRA8875 Flash
*	��    ��: _ucChipType : оƬ���ͣ�FONT_CHIP �� BMP_CHIP
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void WriteRA8875Flash(uint8_t _ucChipType)
{
	uint32_t i;
	uint8_t buf[512], buf_read[512];
	char cDispBuf[32];
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	FIL file;
	uint32_t bw;
	char path[32];
	uint8_t ucVolume = FS_NAND;
	char volume_str[5];
	int32_t iTime1, iTime2;
	uint32_t WriteSize;
	uint32_t OffsetAddr = 0;

	/* ����RA8875����Flash��ص�Ӳ�� */
	bsp_InitRA8875Flash();
	w25_CtrlByMCU();
	w25_SelectChip(_ucChipType);

	/* ������Ҫ�ж�RA8875���Ƿ�֧�ִ���Flash�Ķ�д */
	{
		/* ʶ��Flash�Ƿ�ɶ�д */
		if ((g_tW25.ChipID != W25Q64BV) && (g_tW25.ChipID != W25Q128))
		{
			if ((_ucChipType == FONT_CHIP) && (g_tW25.ChipID == 0))
			{
				/* ��������� */
				w25_ReadBuffer(buf, 0x1FFFF0, 5);

				if (memcmp(buf, "GT20K", 5) == 0)
				{
					/* ��װ�ļ�ͨ�ֿ�оƬ(���Զ�ȡ) */
					FormW25->Label6.pCaption = "��ʾģ����ֿ�оƬ��ֻ����           ";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit1;
				}
				else
				{
					FormW25->Label6.pCaption = "��ʾģ��Ӳ����֧��д�ֿ�оƬ         ";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit1;
				}
			}
			else
			{
				if (_ucChipType == FONT_CHIP)
				{
					FormW25->Label6.pCaption = "��ʾģ��Ӳ����֧��д�ֿ�оƬ";
				}
				else
				{
					FormW25->Label6.pCaption = "��ʾģ��Ӳ����֧��дͼ��оƬ";
				}
				LCD_DrawLabel(&FormW25->Label6);

				goto quit1;
			}
		}
	}

	/* �ļ�ϵͳ���� */
	{
		/* �����ļ�ϵͳ */
		sprintf(volume_str, "%d:", ucVolume);
		result = f_mount(&fs, volume_str, 0);			/* Mount a logical drive */
		if (result != FR_OK)
		{
			printf("�����ļ�ϵͳʧ�� (%d)\r\n", result);

			FormW25->Label6.pCaption = "�����ļ�ϵͳʧ��";
			LCD_DrawLabel(&FormW25->Label6);

			goto quit1;
		}

		/* ���ļ� */
		if (_ucChipType == FONT_CHIP)
		{
			sprintf(path, "%d:/RA8875/FontWrite.bin", ucVolume);

			/* ���ļ� */
			result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
			if (result !=  FR_OK)
			{
				FormW25->Label6.pCaption = "û���ҵ��ļ�FontWrite.bin";
				LCD_DrawLabel(&FormW25->Label6);

				goto quit1;
			}

			/* ����ļ���С */
			{
				FILINFO fno;

				fno.lfname = 0;		/* ��������Ϊ0����Ϊ���ļ���ʹ�ܺ�f_stat�����ڲ���д��������� */
				fno.lfsize = 0;				

				f_stat(path, &fno);
				WriteSize = fno.fsize;

				/* �ļ���С���ܴ��ڴ���Flash������ 8MB */
				if (WriteSize > g_tW25.TotalSize)
				{
					FormW25->Label6.pCaption = "FontWrite.bin�ļ����ȹ���";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit1;
				}
			}

			FormW25->Label6.pCaption = "����д�ֿ�оƬ,���ļ�FontWrite.bin";
			LCD_DrawLabel(&FormW25->Label6);
		}
		else
		{
			sprintf(path, "%d:/RA8875/BmpWrite.bin", ucVolume);

			result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
			if (result !=  FR_OK)
			{
				FormW25->Label6.pCaption = "û���ҵ��ļ�BmpWrite.bin";
				LCD_DrawLabel(&FormW25->Label6);

				goto quit1;
			}

			/* ����ļ���С�� ע��: ���ļ��� */
			{
				FILINFO fno;
				
				fno.lfname = 0;		/* ��������Ϊ0����Ϊ���ļ���ʹ�ܺ�f_stat�����ڲ���д��������� */
				fno.lfsize = 0;

				f_stat(path, &fno);
				WriteSize = fno.fsize;

				/* �ļ���С���ܴ��ڴ���Flash������ 8MB */
				if (WriteSize > g_tW25.TotalSize)
				{
					FormW25->Label6.pCaption = "BmpWrite.bin�ļ����ȹ���";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit1;
				}
			}

			FormW25->Label6.pCaption = "����дͼ��оƬ, �ļ�BmpWrite.bin";
			LCD_DrawLabel(&FormW25->Label6);

			/* ������������һƬ W25Q128��ǰ��2MB���ֿ⡣����14MB��ͼƬ */
			if (g_tW25.ChipID == W25Q128)
			{
				OffsetAddr = PIC_OFFSET;	/* ǰ��2MB������ֿ� */
			}
		}
	}

	iTime1 = bsp_GetRunTime();
	/* ��Label������ʾ���� */
	FormW25->Label2.pCaption = cDispBuf;
	FormW25->Label4.pCaption = cDispBuf;

	for (i = 0; i < WriteSize / 512; i++)
	{
		if ((i % 16) == 0)
		{
			sprintf(cDispBuf, "0x%06X / 0x%06X(%%%02d)  ", i * 512, WriteSize, ((i + 16) * 100) /(WriteSize / 512));
			LCD_DrawLabel(&FormW25->Label2);

			iTime2 = bsp_GetRunTime() - iTime1;
			sprintf(cDispBuf, "%d ��", iTime2 / 1000);
			LCD_DrawLabel(&FormW25->Label4);
		}

		result = f_read(&file, &buf, 512, &bw);
		if (result != FR_OK)
		{
			FormW25->Label6.pCaption = "�ļ���ȡʧ��";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}

		if (((i * 512) % g_tW25.PageSize) == 0)
		{
			w25_EraseSector(OffsetAddr + i * 512);
		}
		w25_WritePage(buf, OffsetAddr + i * 512, 512);		/* д�봮��Flash */
		w25_ReadBuffer(buf_read, OffsetAddr + i * 512, 512);	/* ���ؽ��бȽ� */
		if (memcmp(buf, buf_read, 512) != 0)
		{
			FormW25->Label6.pCaption = "д����Flash����!";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}
	}

	if (_ucChipType == FONT_CHIP)
	{
		FormW25->Label6.pCaption = "д�ֿ�оƬ�ɹ�";
	}
	else
	{
		FormW25->Label6.pCaption = "дͼ��оƬ�ɹ�";
	}
	LCD_DrawLabel(&FormW25->Label6);

quit1:
	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	f_mount(NULL, volume_str, 0);

	w25_CtrlByRA8875();
}

/*
*********************************************************************************************************
*	�� �� ��: EraseNorFlash
*	����˵��: ����NOR Flash
*	��    ��:
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void EraseNorFlash(void)
{
	uint32_t i;
	char cDispBuf[32];
	int32_t iTime1, iTime2;

	FormW25->Label6.pCaption = "���ڲ���NOR FlashоƬ";
	LCD_DrawLabel(&FormW25->Label6);

	iTime1 = bsp_GetRunTime();

	/* ��Label������ʾ���� */
	FormW25->Label2.pCaption = cDispBuf;
	FormW25->Label4.pCaption = cDispBuf;

#if 0	/* ִ����Ƭ���� (58��), �Ͱ�BLOCK����ʱ���ࡣ */
	NOR_StartEraseChip();	/* ��ʼ������ �˹��̺ܳ�,100���� */

	for (;;)
	{
		if (NOR_CheckStatus() == NOR_SUCCESS)
		{
			break;
		}

		iTime2 = bsp_GetRunTime() - iTime1;
		sprintf(cDispBuf, "%d ��           ", iTime2 / 1000);
		LCD_DrawLabel(&FormW25->Label4);

		sprintf(cDispBuf, "%%%d",  ((iTime2 / 1000) * 100) / 59);
		LCD_DrawLabel(&FormW25->Label2);
	}

	FormW25->Label6.pCaption = "����NOR Flash�ɹ�";
	LCD_DrawLabel(&FormW25->Label6);
#else	/* ���� BLOCK ���в��� (57��) */
	for (i = 0; i < NOR_SECTOR_COUNT; i++)
	{
		sprintf(cDispBuf, "0x%06X (%%%02d)  ", i * NOR_SECTOR_SIZE, ((i + 1) * 100) / NOR_SECTOR_COUNT);
		LCD_DrawLabel(&FormW25->Label2);

		iTime2 = bsp_GetRunTime() - iTime1;
		sprintf(cDispBuf, "%d ��      ", iTime2 / 1000);
		LCD_DrawLabel(&FormW25->Label4);

		NOR_EraseSector(i * NOR_SECTOR_SIZE);
	}

	FormW25->Label6.pCaption = "����NOR Flash�ɹ�";
	LCD_DrawLabel(&FormW25->Label6);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: WriteNorFlash
*	����˵��: ���ļ�����д��NOR Flash.  (ִ��ʱ�� 59�� @ 5MB)
*	��    ��:
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void WriteNorFlash(void)
{
	uint32_t i;
	uint8_t buf[512];
	char cDispBuf[32];

	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	FIL file;
	uint32_t bw;
	char path[32];
	uint8_t ucVolume = FS_NAND;
	char volume_str[5];
	int32_t iTime1, iTime2;

	/* �ļ�ϵͳ���� */
	{
		/* �����ļ�ϵͳ */
		sprintf(volume_str, "%d:", ucVolume);
		result = f_mount(&fs, volume_str, 0);			/* Mount a logical drive */
		if (result != FR_OK)
		{
			printf("�����ļ�ϵͳʧ�� (%d)\r\n", result);

			FormW25->Label6.pCaption = "�����ļ�ϵͳʧ��";
			LCD_DrawLabel(&FormW25->Label6);

			goto quit1;
		}

		/* ���ļ� */
		sprintf(path, "%d:/NorFlash/NorFlash.bin", ucVolume);

		result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
		if (result !=  FR_OK)
		{
			FormW25->Label6.pCaption = "û���ҵ��ļ�NorFlash.bin";
			LCD_DrawLabel(&FormW25->Label6);

			goto quit1;
		}

		FormW25->Label6.pCaption = "����дNOR FlashоƬ,���ļ�NorFlash.bin";
		LCD_DrawLabel(&FormW25->Label6);
	}

	iTime1 = bsp_GetRunTime();
	/* ��Label������ʾ���� */
	FormW25->Label2.pCaption = cDispBuf;
	FormW25->Label4.pCaption = cDispBuf;

	/* NOR Flash ֧��ҳ��(64�ֽ�)�������,���ǲ��ܿ�ҳ��� */
	for (i = 0; i < file.fsize / 64; i++)
	{
		if ((i % 10) == 0)
		{
			sprintf(cDispBuf, "0x%06X (%%%02d)  ", i * 64, ((i + 1) * 100) /(file.fsize / 64));
			LCD_DrawLabel(&FormW25->Label2);

			iTime2 = bsp_GetRunTime() - iTime1;
			sprintf(cDispBuf, "%d ��      ", iTime2 / 1000);
			LCD_DrawLabel(&FormW25->Label4);
		}
		result = f_read(&file, &buf, 64, &bw);
		if (result != FR_OK)
		{
			printf("%s �ļ���ȡʧ��\r\n", path);

			FormW25->Label6.pCaption = "�ļ���ȡʧ��";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}
		if (NOR_WriteInPage((uint16_t *)&buf, i * 64, 64 / 2) != NOR_SUCCESS)
		{
			FormW25->Label6.pCaption = "NOR���ʧ��";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}
	}

	sprintf(cDispBuf, "0x%06X (%%%02d)  ", i * 64, 100);
	LCD_DrawLabel(&FormW25->Label2);

	FormW25->Label6.pCaption = "д�ɹ�";
	LCD_DrawLabel(&FormW25->Label6);

quit1:
	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	f_mount(NULL, volume_str, 0);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

/*
*********************************************************************************************************
*
*	ģ������ : �������Խ��档
*	�ļ����� : touch_test.c
*	��    �� : V1.1
*	˵    �� : �������Ƿ���ȱ�ݣ������Ƿ�׼ȷ
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-01-01 armfly  ��ʽ����
*		V1.1    2014-09-06 armfly  ����ͼƬ��ʾ�ٶȲ��Թ���
*
*	Copyright (C), 2014-2015, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"			/* printf����������������ڣ����Ա����������ļ� */

#include "main_menu.h"
#include "images.h"
#include "param.h"
#include "ff.h"				/* FatFS �ļ�ϵͳͷ�ļ� */
//#include "usbh_bsp_msc.h"	/* usb host u��֧�� */

#include "FILE_SYS.H"		/*��CH376 �ļ�ϵͳ */

#define PIC_DISK	FS_NAND		/* ѡ��ȱʡ��ȡ�Ĵ��̣� FS_NAND, FS_SD, FS_USB */
#define PIC_PATH	"/Pic"		/* MP3�ļ���ŵ�ȱʡ�ļ��У� ��Ŀ¼�µ�PIC */

/* �������ṹ */
typedef struct
{
	FONT_T FontBlack;	/* ��̬������ */
	FONT_T FontBlue;	/* �仯���������� */

	BUTTON_T Btn1;		/* ���� */
	BUTTON_T Btn2;		/* �һ� */
	BUTTON_T Btn3;		/* �л���Ƶͨ�� */

	BUTTON_T BtnRet;	/* ���� */

	char strHardInfo[32];		/* ģ��Ӳ����Ϣ */
	uint8_t ucNetStatus;	/* ����״̬ */
	uint8_t ucAudioCh;		/* ��ǰ��Ƶͨ�� 0�� 1 */
	uint8_t ucEarVolume;	/* �������� 0 - 5 */
	int16_t ucMicGain;		/* MIC����  -12����С����  12���������  13������*/

}FormGPRS_T;

/* ���ذ�ť������(��Ļ���½�) */
#define BUTTON_RET_H	32
#define BUTTON_RET_W	60
#define	BUTTON_RET_X	(g_LcdWidth - BUTTON_RET_W - 4)
#define	BUTTON_RET_Y	(g_LcdHeight - BUTTON_RET_H - 4)

static LABEL_T Label1;		/* ��ʾPS2�豸״̬ */
static LABEL_T Label2;		/* ��ʾPS2���̺���귢���ļ�ֵ */
static LABEL_T Label3;		/* ��ʾҡ�ˣ����⣬��ť�¼� */

static FONT_T FontLabel;


/* ͼƬ��ַ RA8875ͼƬоƬƫ�Ƶ�ַ */
const uint32_t g_RA8875Addr_480x272[3] =
{
	0x00200000,	/* 480x272 ɳ̲���� */
	0x0023FC00,	/* 480x272 ��Ҷ���� */
	0x0027F800,	/* 480x272 ˮ�鱳�� */
};
const uint32_t g_RA8875Addr_800x480[3] =
{
	0x002BF400,	/* 800x480 ɳ̲���� */
	0x0037AC00,	/* 800x480 ��Ҷ���� */
	0x00436400	/* 800x480 ˮ�鱳�� */
};

/* NOR FLASH�д洢��ͼƬ��ַ */
const uint32_t g_NorAddr_480x272[3] =
{
	(0x64000000 + 0x00200000),	/* 480x272 ɳ̲���� */
	(0x64000000 + 0x0023FC00),	/* 480x272 ��Ҷ���� */
	(0x64000000 + 0x0027F800),	/* 480x272 ˮ�鱳�� */
};
const uint32_t g_NorAddr_800x480[3] =
{
	(0x64000000 + 0x002BF400),	/* 800x480 ɳ̲���� */
	(0x64000000 + 0x0037AC00), 	/* 800x480 ��Ҷ���� */
	(0x64000000 + 0x00436400)	/* 800x480 ˮ�鱳�� */
};

/* NOR FLASH, PICĿ¼�� �洢��ͼƬ�ļ��� */
const char * g_NandFileName_480x272[3] =
{
	/* CH376 U���ļ�ֻ֧�� 8.3��ʽ���ļ�����д */
	"480272_1.BIN",
	"480272_2.BIN",
	"480272_3.BIN",
	//"480x272_4.bin",
	//"480x272_5.bin",
	//"480x272_6.bin"
};

const char * g_NandFileName_800x480[3] =
{
	"800480_1.BIN",
	"800480_2.BIN",
	"800480_3.BIN",
	//"800x480_4.bin",
	//"800x480_5.bin",
	//"800x480_6.bin"
};

typedef struct
{
	uint32_t code;
	char * str;
}KB_STR_T;

static const KB_STR_T s_KeyNameTab[] =
{
	{0xEEEEEEEE, ""},

    {KB_A, "A"},
    {KB_B, "B"},
    {KB_C, "C"},
    {KB_D, "D"},
    {KB_E, "E"},
    {KB_F, "F"},
    {KB_G, "G"},
    {KB_H, "H"},
    {KB_I, "I"},
    {KB_J, "J"},
    {KB_K, "K"},
    {KB_L, "L"},
    {KB_M, "M"},
    {KB_N, "N"},
    {KB_O, "O"},
    {KB_P, "P"},
    {KB_Q, "Q"},
    {KB_R, "R"},
    {KB_S, "S"},
    {KB_T, "T"},
    {KB_U, "U"},
    {KB_V, "V"},
    {KB_W, "W"},
    {KB_X, "X"},
    {KB_Y, "Y"},
    {KB_Z, "Z"},
    {KB_0, "0"},
    {KB_1, "1"},
    {KB_2, "2"},
    {KB_3, "3"},
    {KB_4, "4"},
    {KB_5, "5"},
    {KB_6, "6"},
    {KB_7, "7"},
    {KB_8, "8"},
    {KB_9, "9"},
    {KB_PIE, "`"},	/* Ʋ���������Ͻ� */
    {KB_SUB, "-"},	/* �иܣ����� */
    {KB_EQU, "="},	/* �Ⱥ� */
    {KB_FXG, "\\"},	/* ��б�� */
    {KB_BKSP,"Backspace"},
    {KB_SPACE, "Space"},
    {KB_TAB, "Tab"},
    {KB_CAPS, "CapsLk"},
    {KB_L_SHFT, "Shift Left"},
    {KB_L_CTRL, "Ctrl Left"},
    {KB_L_GUI, "GUI Left"},
    {KB_L_ALT, "Alt Left"},
    {KB_R_SHFT,"Shift Right"},
    {KB_R_CTRL, "Ctrl Right"},
    {KB_R_GUI, "Gui Right"},
    {KB_R_ALT, "Alt Right"},
    {KB_APPS, "Apps"},
    {KB_ENTER, "Enter"},
    {KB_ESC, "ESC"},
    {KB_F1, "F1"},
    {KB_F2, "F2"},
    {KB_F3, "F3"},
    {KB_F4, "F4"},
    {KB_F5, "F5"},
    {KB_F6, "F6"},
    {KB_F7, "F7"},
    {KB_F8, "F8"},
    {KB_F9, "F9"},
    {KB_F10, "F10"},
    {KB_F11, "F11"},
    {KB_F12, "F12"},
    {KB_PRNT_SCRN, "Print Screen/SysRq"},
    {KB_SCROLL, "Scroll Lock"},
    {KB_PAUSE, "Pause/Break"},
    {KB_ZZKH, "["},
    {KB_INSERT, "Insert"},
    {KB_HOME, "Home"},
    {KB_PGUP, "Page Up"},
    {KB_DELETE, "Delete"},
    {KB_END, "End"},
    {KB_PGDN, "Page Down"},
    {KB_U_ARROW, "Up Arrow"},
    {KB_L_ARROW, "Left Arrow"},
    {KB_D_ARROW, "Down Arrow"},
    {KB_R_ARROW, "Right Arrow"},
    {KB_NUM, "Num Lock"},
    {KB_KP_DIV, "KP /"},    /* С���̳���  KP ��ʾС���� */
    {KB_KP_MULT, "KP *"},	/* С���̳˺� */
    {KB_KP_SUB, "KP -"},	/* - */
    {KB_KP_ADD, "KP +"},
    {KB_KP_ENTER, "KP Enter"},
    {KB_KP_DOT, "KP ."},	/* С���� */
    {KB_KP_0, "KP 0"},
    {KB_KP_1, "KP 0"},
    {KB_KP_2, "KP 0"},
    {KB_KP_3, "KP 0"},
    {KB_KP_4, "KP 0"},
    {KB_KP_5, "KP 0"},
    {KB_KP_6, "KP 0"},
    {KB_KP_7, "KP 0"},
    {KB_KP_8, "KP 0"},
    {KB_KP_9, "KP 0"},
    {KB_YZKH, "]"}, 	/* ] �������� */
    {KB_SEMICOLON, ";"},/* ; �ֺ� */
    {KB_QUOTES, "'"},	/* ������ */
    {KB_COMMA, ","},	/* ���� */
    {KB_DOT, "."},		/* С���� */
    {KB_DIV, "/"},		/* ���� */

	{0, ""}		/* ��������־ */
};

static void InitFormTouch(void);
static const char * GetNameOfKey(uint32_t _code);
static void DispPic(void);
uint8_t ReadFileDispPic(char *_strFileName, uint16_t _usX, uint16_t _usY, uint16_t usHeight, uint16_t usWidth);

static uint8_t s_pic = 0;

/*
*********************************************************************************************************
*	�� �� ��: TestTouch
*	����˵��: ��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TestTouch(void)
{
	uint8_t ucKeyCode;		/* �������� */
	uint32_t uiPS2Msg;		/* PS2���������Ϣ */
	uint8_t ucTouch;		/* �����¼� */
	uint8_t fRefresh;		/* ˢ�������־,1��ʾ��Ҫˢ�� */
	FONT_T tFont, tFontBtn;	/* ����һ������ṹ���������������������� */
	char buf[64];
	uint16_t usAdcX, usAdcY;
	int16_t tpX, tpY;
	uint8_t fQuit = 0;
	uint8_t err_376;

	BUTTON_T tBtn;

	s_pic = 100;

	LCD_ClrScr(CL_BLUE);  	/* ������������ɫ */

	/* ����������� */
	{
		tFont.FontCode = FC_ST_16;	/* ������� 16���� */
		tFont.FrontColor = CL_WHITE;	/* ������ɫ */
		tFont.BackColor = CL_BLUE;	/* ���ֱ�����ɫ */
		tFont.Space = 0;				/* ���ּ�࣬��λ = ���� */
	}

	{
		tFontBtn.FontCode = FC_ST_16;
		tFontBtn.BackColor = CL_MASK;		/* ����͸�� */
		tFontBtn.FrontColor = CL_BLACK;
		tFontBtn.Space = 0;
	}

	LCD_SetBackLight(g_tParam.ucBackLight);			/* ���ñ������� */

	InitFormTouch();

#if 1
	bsp_InitPS2();
	PS2_StartWork();

	/* ��ʼ��PS2�豸. ���̿��Բ�����. ���������ú�Ż�������ݰ� */
	{
		uint8_t ucDevType;

		ucDevType = PS2_GetDevceType();
		if (ucDevType  == PS2_MOUSE)
		{
			Label1.pCaption = "��⵽PS2���";
			LCD_DrawLabel(&Label1);

			PS2_InitMouse();
		}
		else if (ucDevType == PS2_KEYBOARD)
		{
			Label1.pCaption = "��⵽PS2����";
			LCD_DrawLabel(&Label1);

			PS2_InitKeyboard();
		}
		else
		{
			Label1.pCaption = "δ��⵽PS2�豸";
			LCD_DrawLabel(&Label1);
		}
	}
#endif

	IRD_StartWork();	/* ��ʼ������� */

	err_376 = mInitCH376Host( );  /* ��ʼ��CH376 */
	if (err_376 != USB_INT_SUCCESS)
	{
		printf("CH376 Error: %02X\n", err_376 );  /* ��ʾ���� */
	}
	
	fRefresh = 1;	/* 1��ʾ��Ҫˢ��LCD */
	while (fQuit == 0)
	{
		bsp_Idle();

		if (fRefresh)
		{
			fRefresh = 0;

			/* ʵʱˢ�´���ADC����ֵ��ת��������� */
			{
				LCD_DispStr(5, 3, "���Դ�������������ҡ�ˡ�LED��", &tFont);			/* ��(8,3)���괦��ʾһ������ */

				/* ��ʾTFT�������ͺź���Ļ�ֱ��� */
				LCD_GetChipDescribe(buf);	/* ��ȡTFT����оƬ�ͺ� */
				sprintf(&buf[strlen(buf)], "   %d x %d", LCD_GetWidth(), LCD_GetHeight());
				LCD_DispStr(5, 23, buf, &tFont);			/* ��(8,3)���괦��ʾһ������ */

				if (g_ChipID == IC_8875)
				{
					/* ������ɫ���̵׺��� */
					RA8875_SetFrontColor(CL_RED);
					RA8875_SetBackColor(CL_GREEN);

					/* RA8875_DispStr ����������RA8875�ⲿ�ֿ�оƬ��ʾ�ַ�����֧��ASCII�ͺ��� */

					/* ԭ��16���� */
					RA8875_SetFont(RA_FONT_16, 0, 0);
					RA8875_SetTextZoom(RA_SIZE_X1, RA_SIZE_X1);
					RA8875_DispStr(5, 40, "RA8875�����ֿ�оƬOK");
				}

				/* ��ȡ����ʾ��ǰX���Y���ADC����ֵ */
				usAdcX = TOUCH_ReadAdcX();
				usAdcY = TOUCH_ReadAdcY();
				sprintf(buf, "����ADCֵ X = %4d, Y = %4d   ", usAdcX, usAdcY);
				LCD_DispStr(5, 60, buf, &tFont);

				/* ��ȡ����ʾ��ǰ�������� */
				tpX = TOUCH_GetX();
				tpY = TOUCH_GetY();
				sprintf(buf, "��������  X = %4d, Y = %4d   ", tpX, tpY);
				LCD_DispStr(5, 80, buf, &tFont);

				/* �ڴ�������λ����ʾһ��СȦ */
				if ((tpX > 0) && (tpY > 0))
				{
					LCD_DrawCircle(tpX, tpY, 2, CL_YELLOW);
				}
			}

			/* ��ʾͼ������ */
			{
				tBtn.Font = &tFontBtn;

				tBtn.Left = BUTTON_RET_X;
				tBtn.Top = BUTTON_RET_Y;
				tBtn.Height = BUTTON_RET_H;
				tBtn.Width = BUTTON_RET_W;
				tBtn.Focus = 0;	/* ʧȥ���� */
				tBtn.pCaption = "����";
				LCD_DrawButton(&tBtn);
			}

			/* ����Ļ���ػ���2�����ο�(���ڼ������Ե�����Ƿ�����) */
			LCD_DrawRect(0, 0, LCD_GetHeight(), LCD_GetWidth(), CL_WHITE);
			LCD_DrawRect(2, 2, LCD_GetHeight() - 4, LCD_GetWidth() - 4, CL_YELLOW);

			/* ��ʾ����ֵ */
			sprintf(buf, "��ǰ��������: %3d", g_tParam.ucBackLight);
			LCD_DispStr(5, 130, buf, &tFont);

			if (g_ChipID == IC_8875)
			{
				/* 3.5������֧�� */
				LCD_DispStr(5, LCD_GetHeight() - 20, "��K1 K2 ����ʾͼƬ", &tFont);
			}
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* ��ȡ�����¼� */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* ���ʰ����¼� */
					if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
					{
					    tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 1;	/* ���� */
						tBtn.pCaption = "����";
						LCD_DrawButton(&tBtn);
					}

					/* �ڴ�������λ����ʾһ��СȦ */
					if ((tpX > 0) && (tpY > 0))
					{
						LCD_DrawCircle(tpX, tpY, 3, CL_RED);
						BUTTON_BEEP();	/* ������ʾ�� */
					}
					break;

				case TOUCH_MOVE:		/* �����ƶ��¼� */
					/* ʵʱˢ�´���ADC����ֵ��ת��������� */
					{
						/* ��ȡ����ʾ��ǰX���Y���ADC����ֵ */
						usAdcX = TOUCH_ReadAdcX();
						usAdcY = TOUCH_ReadAdcY();
						sprintf(buf, "����ADCֵ X = %4d, Y = %4d   ", usAdcX, usAdcY);
						LCD_DispStr(5, 60, buf, &tFont);

						/* ��ȡ����ʾ��ǰ�������� */
						//tpX = TOUCH_GetX();
						//tpY = TOUCH_GetY();
						sprintf(buf, "��������  X = %4d, Y = %4d   ", tpX, tpY);
						LCD_DispStr(5, 80, buf, &tFont);

						/* �ڴ�������λ����ʾһ��СȦ */
						if ((tpX > 0) && (tpY > 0))
						{
							LCD_DrawCircle(tpX, tpY, 2, CL_YELLOW);
						}
					}
					break;

				case TOUCH_RELEASE:		/* �����ͷ��¼� */
					/* �ڴ�������λ����ʾһ��СȦ */
					if ((tpX > 0) && (tpY > 0))
					{
						LCD_DrawCircle(tpX, tpY, 4, CL_WHITE);
					}

					if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 1;	/* ���� */
						tBtn.pCaption = "����";
						LCD_DrawButton(&tBtn);

						SaveParam();	/* ������� */
						fQuit = 1;
						break;		/* ���� */
					}
					else	/* ��ťʧȥ���� */
					{
						tBtn.Font = &tFontBtn;

						tBtn.Left = BUTTON_RET_X;
						tBtn.Top = BUTTON_RET_Y;
						tBtn.Height = BUTTON_RET_H;
						tBtn.Width = BUTTON_RET_W;
						tBtn.Focus = 0;	/* ���� */
						tBtn.pCaption = "����";
						LCD_DrawButton(&tBtn);

					}
			}
		}

		ucKeyCode = bsp_GetKey();	/* ��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* �м����� */
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1������ */
					bsp_LedOn(1);	/* ����LED1 */

					if (g_ChipID == IC_8875)
					{
						if (s_pic == 100)
						{
							s_pic = 0;
						}
						else if (s_pic > 0)
						{
							s_pic--;
						}
						else
						{
							s_pic = 14;
						}
						DispPic();		/* ��ʾͼƬ */
					}
					break;

				case KEY_UP_K1:				/* K1���ɿ� */
					//Label3.pCaption = "K1���ɿ�";
					//LCD_DrawLabel(&Label3);
					bsp_LedOff(1);	/* Ϩ��LED1 */
					break;

				case KEY_DOWN_K2:			/* K2������ */
					//Label3.pCaption = "K2������";
					//LCD_DrawLabel(&Label3);
					bsp_LedOn(2);	/* ����LED2 */
					if (g_ChipID == IC_8875)
					{
						if (s_pic == 100)
						{
							s_pic = 0;
						}
						else if (++s_pic > 14)
						{
							s_pic = 0;
						}
						DispPic();		/* ��ʾ��һ��ͼƬ */
					}
					break;

				case KEY_UP_K2:				/* K2���ɿ� */
					//Label3.pCaption = "K2���ɿ�";
					//LCD_DrawLabel(&Label3);
					bsp_LedOff(2);	/* Ϩ��LED2 */
					break;

				case KEY_DOWN_K3:			/* K3������ */
					Label3.pCaption = "K3������";
					LCD_DrawLabel(&Label3);
					bsp_LedOn(3);	/* ����LED3 */
					break;

				case KEY_UP_K3:				/* K3���ɿ� */
					Label3.pCaption = "K3���ɿ�";
					LCD_DrawLabel(&Label3);
					bsp_LedOff(3);	/* Ϩ��LED3 */
					break;

				case JOY_DOWN_U:			/* ҡ��UP������ */
					Label3.pCaption = "ҡ���ϼ�����";
					LCD_DrawLabel(&Label3);

					if (g_tParam.ucBackLight < 255)
					{
						g_tParam.ucBackLight++;
					}
					LCD_SetBackLight(g_tParam.ucBackLight);		/* ���ñ������� */
					fRefresh = 1;
					break;

				case JOY_DOWN_D:			/* ҡ��DOWN������ */
					Label3.pCaption = "ҡ���¼�����";
					LCD_DrawLabel(&Label3);
					if (g_tParam.ucBackLight > 0)
					{
						g_tParam.ucBackLight--;
					}
					LCD_SetBackLight(g_tParam.ucBackLight);		/* ���ñ������� */
					fRefresh = 1;
					break;

				case JOY_DOWN_L:			/* ҡ��LEFT������ */
					Label3.pCaption = "ҡ���������";
					LCD_DrawLabel(&Label3);
					break;

				case JOY_DOWN_R:			/* ҡ��RIGHT������ */
					Label3.pCaption = "ҡ���Ҽ�����";
					LCD_DrawLabel(&Label3);
					bsp_DelayMS(500);
					fQuit = 1;
					break;

				case JOY_DOWN_OK:			/* ҡ��OK������ */
					Label3.pCaption = "ҡ��OK������";
					LCD_DrawLabel(&Label3);
					bsp_LedOn(4);	/* ����LED4 */
					break;

				case JOY_UP_OK:				/* ҡ��OK���ɿ� */
					Label3.pCaption = "ҡ��OK���ɿ�";
					LCD_DrawLabel(&Label3);
					bsp_LedOff(4);	/* Ϩ��LED4 */
					break;

				/* �����Ǻ���ң�������¼� */
				case IR_KEY_POWER:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR Power";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_MENU:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR Menu";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_TEST:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR Test";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_UP:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR +";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_RETURN:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR Return";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_LEFT:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR Left";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_OK:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR Ok";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_RIGHT:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR Right";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_0:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 0";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_DOWN:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR -";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_C:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR C";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_1:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 1";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_2:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 2";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_3:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 3";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_4:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 4";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_5:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 5";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_6:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 6";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_7:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 7";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_8:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 8";
					LCD_DrawLabel(&Label3);
					break;

				case IR_KEY_9:
					BUTTON_BEEP();	/* ������ʾ�� */
					Label3.pCaption = "IR 9";
					LCD_DrawLabel(&Label3);
					break;

				default:
					#if 0
					if (ucKeyCode >= IR_KEY_STRAT)
					{
						sprintf(buf, "IR �޷�ʶ�� %02X %02X %02X %02X", g_tIR.RxBuf[0], g_tIR.RxBuf[1],
							g_tIR.RxBuf[2], g_tIR.RxBuf[3]);

						Label3.pCaption = buf;
						LCD_DrawLabel(&Label3);
					}
					#endif
					break;

			}
		}

		/* PS2����������жϷ������ʵ�֣�����ֻ��Ҫ����PS2_GetMsg��ȡ��ֵ���ɡ� */
		uiPS2Msg = PS2_GetMsg();
		if (uiPS2Msg != PS2_NONE)
		{
			//sprintf(buf, "%X\r\n", uiPS2Msg);
			//Label1.pCaption = buf;
			//LCD_DrawLabel(&Label1);

			if (uiPS2Msg == 0xAA)
			{
				Label1.pCaption = "��⵽PS2����";
				LCD_DrawLabel(&Label1);

				PS2_InitKeyboard();
			}
			if (uiPS2Msg == 0xAA00)
			{
				Label1.pCaption = "��⵽PS2���";
				LCD_DrawLabel(&Label1);

				PS2_InitMouse();
			}

			/* ��ӡPS2������ݰ������� */
			if (PS2_IsMousePacket(uiPS2Msg))
			{
				MOUSE_PACKET_T mouse;

				PS2_DecodeMouse(uiPS2Msg, &mouse);

				sprintf(buf,  "X=%4d, Y=%4d, Z=%2d, ���=%d, �м�=%d, �Ҽ�=%d",
					mouse.Xmove, mouse.Ymove, mouse.Zmove,
					mouse.BtnLeft, mouse.BtnMid, mouse.BtnRight);

				Label2.pCaption = buf;
				LCD_DrawLabel(&Label2);
			}
			else	/* ��ΪPS2�������ݰ����� */
			{
				sprintf(buf, "%s", GetNameOfKey(uiPS2Msg));
				Label2.pCaption = buf;
				LCD_DrawLabel(&Label2);
			}
		}
	}

	PS2_StopWork();		/* ֹͣPS2�ж� */
	IRD_StopWork();	/* ֹͣ������� */
}

/*
*********************************************************************************************************
*	�� �� ��: DispPic
*	����˵��: ������ʾRA8875ͼƬ��3�ţ���NORͼƬ��2�ţ���SRAMͼƬ��1�ţ���NANDͼƬ��6�ţ���������ˢ��ʱ��
*	��    ��: ��
*	�� �� ֵ: �ַ���ָ��
*********************************************************************************************************
*/
static void DispPic(void)
{
	char buf[64];

	int32_t time1, time2;
	FONT_T tFont;

	/* ����������� */
	{
		tFont.FontCode = FC_ST_16;	/* ������� 16���� */
		tFont.FrontColor = CL_WHITE;	/* ������ɫ */
		tFont.BackColor = CL_BLUE;	/* ���ֱ�����ɫ */
		tFont.Space = 0;				/* ���ּ�࣬��λ = ���� */
	}

	/* K1���л�����ͼƬ */
	if (s_pic < 3)	/* RA8875 ͼ��оƬ3��ͼƬ */
	{
		/*  ��ʾRA8875���ͼ��оƬ�е�ͼ��ʹ������ĺ���
		void RA8875_DispBmpInFlash(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth,
				uint32_t _uiFlashAddr);
		*/
		time1 = bsp_GetRunTime();
		if (LCD_GetWidth() == 480)	/* 4.3���� 480x272 */
		{
			RA8875_DispBmpInFlash(0, 0, 272, 480, g_RA8875Addr_480x272[s_pic]);
		}
		else	/* 5���7�� 800*480 �� */
		{
			RA8875_DispBmpInFlash(0, 0, 480, 800, g_RA8875Addr_800x480[s_pic]);
		}
		time2 = bsp_GetRunTime();
		sprintf(buf, "ͼƬ%d RA8875    , ��ʾʱ��: %4dms", s_pic + 1, time2 - time1);
		LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);
	}
	else if (s_pic < 5)	/* NOR FLASH ��ͼƬ2�� */
	{
		time1 = bsp_GetRunTime();
		if (LCD_GetWidth() == 480)	/* 4.3���� 480x272 */
		{
			LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, (uint16_t *)g_NorAddr_480x272[s_pic - 3]);
		}
		else	/* 5���7�� 800*480 �� */
		{
			LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, (uint16_t *)g_NorAddr_800x480[s_pic - 3]);
		}
		time2 = bsp_GetRunTime();
		sprintf(buf, "ͼƬ%d NOR FLASH, ��ʾʱ��: %4dms", s_pic +1 , time2 - time1);
		LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);

		/*	��NOR�е�3��ͼƬ���Ƶ��ⲿSRAM�� ׼������SRAM��ͼƬˢ��ʱ�� */
		{
			/* �ⲿSRAMֻ�ܴ洢2��800*480��ͼƬ */
			uint16_t *p1 = (uint16_t *)EXT_SRAM_ADDR;
			uint32_t i;

			if (LCD_GetWidth() == 480)	/* 4.3���� 480x272 */
			{
				for (i = 0; i < g_LcdHeight * g_LcdWidth; i++)
				{
					p1[i] = ((uint16_t *)g_NorAddr_480x272[2])[i];
				}
			}
			else	/* 5���7�� 800*480 �� */
			{
				for (i = 0; i < g_LcdHeight * g_LcdWidth; i++)
				{
					p1[i] = ((uint16_t *)g_NorAddr_800x480[2])[i];
				}
			}
		}
	}
	else if (s_pic < 6)	/* SRAM ��ͼƬ1�� */
	{
		uint16_t *p1 = (uint16_t *)EXT_SRAM_ADDR;

		time1 = bsp_GetRunTime();
		if (LCD_GetWidth() == 480)	/* 4.3���� 480x272 */
		{
			LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, p1);
		}
		else	/* 5���7�� 800*480 �� */
		{
			LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, p1);
		}
		time2 = bsp_GetRunTime();
		sprintf(buf, "ͼƬ%d SRAM, ��ʾʱ��: %4dms", s_pic + 1, time2 - time1);
		LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);
	}
	else if (s_pic < 9)			/* NAND FLASH �е�ͼƬ3�� */
	{
		/* ����Fatfs�õ���ȫ�ֱ��� */
		FATFS   fs;
		char FileName[64];
		uint8_t err = 0;

		/* ÿ�ζ�1�� */
		if (f_mount(&fs, FS_VOLUME_NAND, 0) != FR_OK)
		{
			printf("f_mount�ļ�ϵͳʧ��");
		}

		time1 = bsp_GetRunTime();

		if (LCD_GetWidth() == 480)	/* 4.3���� 480x272 */
		{
			sprintf(FileName, "%d:%s/%s", PIC_DISK, PIC_PATH, g_NandFileName_480x272[s_pic - 6]);
		}
		else
		{
			sprintf(FileName, "%d:%s/%s", PIC_DISK, PIC_PATH, g_NandFileName_800x480[s_pic - 6]);
		}

		/* �������е�ͼƬ�ļ�������ʾ */
		if (ReadFileDispPic(FileName, 0, 0, g_LcdHeight, g_LcdWidth) == 0)
		{
			err = 1;
		}

		time2 = bsp_GetRunTime();

		if (err == 1)
		{
			LCD_ClrScr(CL_BLUE);
			sprintf(buf, "ͼƬ%d NAND FLASH, �ļ�����: %dms", s_pic + 1, 0);
		}
		else
		{
			sprintf(buf, "ͼƬ%d NAND FLASH, ��ʾʱ��: %dms", s_pic + 1, time2 - time1);
		}
		LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);

		/* ж���ļ�ϵͳ */
		f_mount(NULL, FS_VOLUME_NAND, 0);
	}
	else if (s_pic < 12)			/* SD���е�ͼƬ3�� */
	{
		/* ����Fatfs�õ���ȫ�ֱ��� */
		FATFS   fs;
		char FileName[64];
		uint8_t err = 0;

		/* ÿ�ζ�1�� */
		if (f_mount(&fs, FS_VOLUME_SD, 0) == FR_OK)
		{
			time1 = bsp_GetRunTime();

			if (LCD_GetWidth() == 480)	/* 4.3���� 480x272 */
			{
				sprintf(FileName, "%d:Pic/%s", FS_SD, g_NandFileName_480x272[s_pic - 9]);
			}
			else
			{
				sprintf(FileName, "%d:Pic/%s", FS_SD, g_NandFileName_800x480[s_pic - 9]);
			}
			if (ReadFileDispPic(FileName, 0, 0, g_LcdHeight, g_LcdWidth) == 0)
			{
				err = 1;
			}

			time2 = bsp_GetRunTime();
		}
		else
		{
			err = 1;
		}


		if (err == 1)
		{
			LCD_ClrScr(CL_BLUE);
			sprintf(buf, "ͼƬ%d SD���ļ�����: %s", s_pic + 1, FileName);
		}
		else
		{
			sprintf(buf, "ͼƬ%d SD��, ��ʾʱ��: %dms", s_pic + 1, time2 - time1);
		}
		LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);

		/* ж���ļ�ϵͳ */
		f_mount(NULL, FS_VOLUME_SD, 0);
	}
	else if (s_pic < 15)			/* ch376  U���е�ͼƬ3�� */
	{
		uint8_t err = 0;
		uint8_t err_376 = 0;
		char FileName[64];
		uint16_t data[800];
		
		err_376 = CH376DiskConnect();
		if (err_376 != USB_INT_SUCCESS)
		{
			err = 1;
		}
		
		err_376 = CH376DiskMount();  /* ��ʼ�����̲����Դ����Ƿ���� */
		if (err_376 != USB_INT_SUCCESS)
		{
			err = 1;
		}
	
		if (err == 0)
		{
			time1 = bsp_GetRunTime();

			if (LCD_GetWidth() == 480)	/* 4.3���� 480x272 */
			{
				sprintf(FileName, "/PIC/%s", g_NandFileName_480x272[s_pic - 12]);
			}
			else
			{
				sprintf(FileName, "/PIC/%s", g_NandFileName_800x480[s_pic - 12]);
			}
			
			/* ÿ�ζ�1�� */
			err_376 = CH376FileOpenPath((uint8_t *)FileName );  /* ���ļ�,���ļ���C51��Ŀ¼�� */
			if (err_376 == ERR_MISS_DIR ||err_376 == ERR_MISS_FILE)
			{
				/* û���ҵ��ļ� */
				err = 1;
			}
			else	
			{
				uint16_t i, k;
				uint16_t RealCount;
				uint16_t x, y;
				uint16_t width;
				
				x = 0;
				y = 0;
				width = LCD_GetWidth();
				for (i = 0; i < LCD_GetHeight(); i++)
				{
					for (k = 0; k < width / 40; k++)
					{
						CH376ByteRead((uint8_t *)&data[40 * k], 80, &RealCount);  /* ���ֽ�Ϊ��λ�ӵ�ǰλ�ö�ȡ���ݿ� */					
						if (RealCount != 80)
						{
							err = 1;
							break;
						}
					}
					
					LCD_DrawBMP(x, y + i, 1, width, data);
				}
			}
			
			time2 = bsp_GetRunTime();
		}
		if (err == 1)
		{
			LCD_ClrScr(CL_BLUE);
			sprintf(buf, "ͼƬ%d U���ļ�����:%s", s_pic + 1, FileName);
		}
		else
		{
			sprintf(buf, "ͼƬ%d U��, ��ʾʱ��: %dms", s_pic + 1, time2 - time1);
		}
		LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);

		/* ж���ļ�ϵͳ */
		f_mount(NULL, FS_VOLUME_USB, 0);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: ReadFileDispPic
*	����˵��: ��ͼƬ�ļ�������ʾ�� �ڵ��ñ�����ǰ����ȷ�� �����Ѿ�ִ�� f_mount
*	��    ��: _strFileName : �ļ�ȫ���������̺ź�ȫ·��
*	�� �� ֵ: 0 ��ʾ���� 1��ʾ�ɹ�
*********************************************************************************************************
*/
uint8_t  ReadFileDispPic(char *_strFileName, uint16_t _usX, uint16_t _usY, uint16_t usHeight, uint16_t usWidth)
{
	/* ����Fatfs�õ���ȫ�ֱ��� */
//	FATFS   fs;
	FIL 	file;
	uint32_t bw;
#if 1	
	uint16_t data[800];  	// <---- �ڴ治�㣬ʹ���ⲿsram
#else
	uint16_t *data = (uint16_t *)EXT_SRAM_ADDR;
#endif
	
	if (usWidth > 800)
	{
		return 0;
	}

	/* ���ļ� */
	{
		FRESULT result;
		uint16_t i;

		result = f_open(&file, _strFileName, FA_OPEN_EXISTING | FA_READ);
		if (result !=  FR_OK)
		{
			printf("Open file Error, %s\r\n", _strFileName);
			goto err_ret;
		}

		// f_lseek(&file, 0);	/* �޸��ļ���ǰָ�뵽�ļ�ͷ, ��ͷ��ʼ���� ���Բ�����ȱʡ����0 */

		for (i = 0; i < usHeight; i++)
		{
			f_read(&file, &data, usWidth * 2, &bw);
			if (bw <= 0)
			{
				goto err_ret;
			}

			//LCD_DrawBMP(_usX, _usY, usHeight, usWidth, data);
			LCD_DrawBMP(_usX, _usY + i, 1, usWidth, data);
		}
	}

	/* �ر��ļ�*/
	f_close(&file);
	return 1;	/* ����OK */

err_ret:
	/* �ر��ļ�*/
	f_close(&file);
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: DispKeyBoard
*	����˵��: ��ʾ����������
*	��    ��: ��
*	�� �� ֵ: �ַ���ָ��
*********************************************************************************************************
*/
static const char * GetNameOfKey(uint32_t _code)
{
	uint16_t i = 0;


	while (1)
	{
		if (s_KeyNameTab[i].code == 0)
		{
			break;
		}

		if (_code == s_KeyNameTab[i].code)
		{
			return s_KeyNameTab[i].str;
		}
		i++;
	}

	return "";
}

/*
*********************************************************************************************************
*	�� �� ��: InitFormTouch
*	����˵��: ��ʾ����������
*	��    ��: ��
*	�� �� ֵ: �ַ���ָ��
*********************************************************************************************************
*/
static void InitFormTouch(void)
{
	FontLabel.FontCode = FC_ST_16;
	FontLabel.BackColor = CL_BLUE;
	FontLabel.FrontColor = CL_WHITE;
	FontLabel.Space = 0;

	Label3.Left = 5;
	Label3.Top = 110;
	Label3.MaxLen = 0;
	Label3.pCaption = "";
	Label3.Font = &FontLabel;

	Label1.Left = 5;
	Label1.Top = 150;
	Label1.MaxLen = 0;
	Label1.pCaption = "";
	Label1.Font = &FontLabel;

	Label2.Left = 5;
	Label2.Top = 170;
	Label2.MaxLen = 0;
	Label2.pCaption = "";
	Label2.Font = &FontLabel;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

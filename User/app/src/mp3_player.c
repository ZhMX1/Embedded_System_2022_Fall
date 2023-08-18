/*
*********************************************************************************************************
*
*	ģ������ : MP3����������
*	�ļ����� : "mp3_player.h"
*	��    �� : V1.1
*	˵    �� : ����VS1053 MP3ģ��
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		V1.0    2013-02-01 armfly  �׷�
*		V1.1    2015-10-17 armfly  f_opendir�����f_closedir
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "mp3_player.h"
#include "ff.h"				/* FatFS �ļ�ϵͳͷ�ļ� */

/* �Զ�����ָ������ָ��Ŀ¼�µ�MP3�ļ� 
#define FS_VOLUME_SD		"0:"
#define FS_VOLUME_NAND		"1:"
#define FS_VOLUME_USB		"2:"
*/

#if 0
	#define MP3_DISK	FS_VOLUME_SD		/* ѡ��ȱʡ��ȡ�Ĵ���:  0��SD�� */
	#define MP3_PATH	"0:/Music"			/* MP3�ļ���ŵ�ȱʡ�ļ��У� ��Ŀ¼�µ�Music */
#else
	#define MP3_DISK	FS_VOLUME_NAND		/* ѡ��ȱʡ��ȡ�Ĵ���:  1��NAND */
	#define MP3_PATH	"1:/Music"			/* MP3�ļ���ŵ�ȱʡ�ļ��У� ��Ŀ¼�µ�Music */
	
#endif

#define SONG_LIST_MAX	10			/* �����б������Ŀ */

/* �������ṹ */
typedef struct
{
	FONT_T FontBlack;	/* ��̬������ */
	FONT_T FontBlue;	/* �仯���������� */
	FONT_T FontBtn;		/* ��ť������ */
	FONT_T FontBox;		/* ������������ */

	GROUP_T Box1;

	LABEL_T Label1;	LABEL_T Label2;
	LABEL_T Label3; LABEL_T Label4;
	LABEL_T Label5; LABEL_T Label6;
	LABEL_T Label7; LABEL_T Label8;

	BUTTON_T BtnRet;

	PLAY_LIST_T List[SONG_LIST_MAX];	

	/* ����Fatfs�õ���ȫ�ֱ��� */
	FATFS   fs;
	FIL 	file;
}FormMP3_T;

/* ���屳��ɫ */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* �������ʹ�С */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	(g_LcdHeight - BOX1_Y - 10)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"MP3ģ����Գ���"

/* ���ذ�ť������(��Ļ���½�) */
#define BTN_RET_H	32
#define BTN_RET_W	60
#define	BTN_RET_X	((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define	BTN_RET_Y	((BOX1_Y  + BOX1_H) - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"����"

#define LABEL1_X  	(BOX1_X + 6)
#define LABEL1_Y	(BOX1_Y + 20)
#define LABEL1_TEXT	"оƬ�ͺ� :"

	#define LABEL2_X  	(LABEL1_X + 90)
	#define LABEL2_Y	LABEL1_Y
	#define LABEL2_TEXT	"  "

#define LABEL3_X  	(LABEL1_X)
#define LABEL3_Y	(LABEL1_Y + 20)
#define LABEL3_TEXT	"������� :"

	#define LABEL4_X  	(LABEL3_X + 90)
	#define LABEL4_Y	(LABEL3_Y)
	#define LABEL4_TEXT	" "

#define LABEL5_X  	(LABEL1_X)
#define LABEL5_Y	(LABEL1_Y + 20 * 2)
#define LABEL5_TEXT	"���Ž��� : "

	#define LABEL6_X  	(LABEL5_X + 90)
	#define LABEL6_Y	LABEL5_Y
	#define LABEL6_TEXT	" "

#define LABEL7_X  	(LABEL1_X)
#define LABEL7_Y	(LABEL1_Y + 20 * 3)
#define LABEL7_TEXT	"�ļ���   : "

	#define LABEL8_X  	(LABEL7_X + 90)
	#define LABEL8_Y	LABEL7_Y
	#define LABEL8_TEXT	"---"

static void InitFormMP3(void);
static void DispFormMP3(void);
static void FillSongList(void);
static void MP3HardInit(void);
static void ViewDir(char *_path);
static uint8_t Mp3Pro(void);
static void Mp3DispStatus(void);

FormMP3_T *FormMP3;

MP3_T g_tMP3;

/*
*********************************************************************************************************
*	�� �� ��: ReserveFunc
*	����˵��: �������ܡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MP3Player(void)
{
	uint8_t ucKeyCode;		/* �������� */
	uint8_t ucTouch;		/* �����¼� */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	FormMP3_T form;
	uint8_t ucRefresh;
	uint8_t ucNextSong;

	FormMP3 = &form;
	
	InitFormMP3();
	DispFormMP3();
	MP3HardInit();		/* ����VS1053BӲ����WM8978Ӳ�� */

	/* �ļ�ϵͳ��������0 */
	memset(&FormMP3->fs, 0, sizeof(FormMP3->fs));
	memset(&FormMP3->file, 0, sizeof(FormMP3->file));
	
	/* �����ļ�ϵͳ */
	if (f_mount(&FormMP3->fs, MP3_DISK, 0) != FR_OK)
	{
		printf("f_mount�ļ�ϵͳʧ��");
	}

	/* ��ӡNAND Flash ��Ŀ¼��MP3Ŀ¼�µ��ļ� */
	{
		char path[60];

		sprintf(path, "%s", MP3_DISK);		/* ��Ŀ¼ */
		ViewDir(path);

		sprintf(path, "%s", MP3_PATH);	/* ����Ŀ¼ */
		ViewDir(path);
	}

	FillSongList();		/* ����NAND Flash ��Ŀ¼�µ�MP3�ļ�������䵽�����б����� */

	ucRefresh = 1;
	g_tMP3.ucPauseEn = 0;	/* ȱʡ��ʼ���� */

	if (g_tMP3.ListCount > 0)
	{
		ucNextSong = 1;			/* ��λ��һ�׸����ı�־ */
	}
	else
	{
		ucNextSong = 0;
	}
	g_tMP3.ListIndex = 0;
	bsp_StartAutoTimer(0, 100);		/* 100ms ˢ�½����� */
	/* ����������ѭ���� */
	while (fQuit == 0)
	{
		bsp_Idle();

		/* �򿪸����ļ� */
		if (ucNextSong == 1)
		{
			ucNextSong = 0;

			/* �ر���һ���ļ�*/
			f_close(&FormMP3->file);

			printf("\r\n\r\n");		/* ��һ����ʾ */

			/* ��MUSICĿ¼�µ�mp3 �ļ� */
			{
				char FileName[256];
				FRESULT result;

				sprintf(FileName, "%s/%s", MP3_PATH, FormMP3->List[g_tMP3.ListIndex].FileName);
				result = f_open(&FormMP3->file, FileName, FA_OPEN_EXISTING | FA_READ);
				if (result !=  FR_OK)
				{
					printf("Open MP3 file Error, %s\r\n", FormMP3->List[g_tMP3.ListIndex].FileName);
					FormMP3->Label8.pCaption = "��MP3�ļ�ʧ��";
					LCD_DrawLabel(&FormMP3->Label8);
				}
				else
				{
					printf("���ڲ���: %s\r\n", FormMP3->List[g_tMP3.ListIndex].FileName);

					/* ��ʾ�ļ��� */
					FormMP3->Label8.pCaption = FormMP3->List[g_tMP3.ListIndex].FileName;
					LCD_DrawLabel(&FormMP3->Label8);
				}
			}

			g_tMP3.uiProgress = 0;	/* ���� */
		}

		if (g_tMP3.ucPauseEn == 0)
		{
			if (Mp3Pro() == 1)
			{
				/* ����������ϣ��Զ��л�����һ�׸� */

				/* ģ��һ��ҡ���Ҽ����£�ִ�л������ */
				bsp_PutKey(JOY_DOWN_R);
			}
		}

		/* ˢ��״̬�� */
		if ((ucRefresh == 1) || (bsp_CheckTimer(0)))
		{
			ucRefresh = 0;
			Mp3DispStatus();		/* ��ʾ��ǰ״̬�������� */
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* ��ȡ�����¼� */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* ���ʰ����¼� */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormMP3->BtnRet.Focus = 1;
						LCD_DrawButton(&FormMP3->BtnRet);
					}
					break;

				case TOUCH_RELEASE:		/* �����ͷ��¼� */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormMP3->BtnRet.Focus = 0;
						LCD_DrawButton(&FormMP3->BtnRet);
						fQuit = 1;	/* ���� */
					}
					else	/* ��ťʧȥ���� */
					{
						FormMP3->BtnRet.Focus = 0;
						LCD_DrawButton(&FormMP3->BtnRet);
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
				case KEY_DOWN_K1:			/* K1������ */
					if (g_tMP3.ucPauseEn == 0)
					{
						g_tMP3.ucPauseEn = 1;
					}
					else
					{
						g_tMP3.ucPauseEn = 0;
					}
					ucRefresh = 1;
					break;

				case KEY_DOWN_K2:			/* K2������ */
					f_lseek(&FormMP3->file, 0);	/* �޸��ļ���ǰָ�뵽�ļ�ͷ, ��ͷ��ʼ���� */
					g_tMP3.uiProgress = 0;	/* ���� */
					ucRefresh = 1;
					break;

				case KEY_DOWN_K3:			/* K3������ */
					if (g_tMP3.ucMuteOn == 1)
					{
						g_tMP3.ucMuteOn = 0;
						vs1053_SetBASS(0, 0, 0, 0);		/* ԭ��,��������ǿ */
					}
					else
					{
						g_tMP3.ucMuteOn = 1;
						vs1053_SetBASS(0, 0, 10, 100);	/* ���õ�����ǿ(��ֹƵ��100Hz)���������� */
					}
					ucRefresh = 1;
					break;

				case JOY_DOWN_D:		/* ҡ��DOWN������ */
					if (g_tMP3.ucVolume > VS_VOL_MIN)
					{
						g_tMP3.ucVolume--;
						vs1053_SetVolume(g_tMP3.ucVolume);
						ucRefresh = 1;
					}
					break;

				case JOY_DOWN_U:		/* ҡ��UP������ */
					if (g_tMP3.ucVolume < VS_VOL_MAX)
					{
						g_tMP3.ucVolume++;
						vs1053_SetVolume(g_tMP3.ucVolume);
						ucRefresh = 1;
					}
					break;

				case JOY_DOWN_L:		/* ҡ��LEFT������ */
					if (g_tMP3.ListIndex > 0)
					{
						g_tMP3.ListIndex--;		/* ǰһ�׸� */
					}
					else
					{
						g_tMP3.ListIndex = g_tMP3.ListCount - 1;	/* ѭ�� */
					}
					ucRefresh = 1;
					ucNextSong = 1;	 /* ����һ�׸��� */
					break;

				case JOY_DOWN_R:		/* ҡ��RIGHT������ */
					if (g_tMP3.ListIndex < g_tMP3.ListCount - 1)
					{
						g_tMP3.ListIndex++;	/* ��һ�׸� */
					}
					else
					{
						g_tMP3.ListIndex = 0;	/* ѭ�� */
					}
					ucRefresh = 1;
					ucNextSong = 1;	 /* ����һ�׸��� */
					break;

				case JOY_DOWN_OK:		/* ҡ��OK������ */
					ucRefresh = 1;
					break;

				default:
					break;
			}
		}
	}

	bsp_StopTimer(0);	/* �ر��Զ���ʱ�� */

	/* �ر��ļ�*/
	f_close(&FormMP3->file);

	/* ж���ļ�ϵͳ */
	f_mount(NULL, MP3_DISK, 0);
}

/*
*********************************************************************************************************
*	�� �� ��: InitFormMP3
*	����˵��: ��ʼ���ؼ�����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitFormMP3(void)
{
	/* ������������ */
	FormMP3->FontBox.FontCode = FC_ST_16;
	FormMP3->FontBox.BackColor = CL_BTN_FACE;	/* �ͱ���ɫ��ͬ */
	FormMP3->FontBox.FrontColor = CL_BLACK;
	FormMP3->FontBox.Space = 0;

	/* ����1 ���ھ�ֹ��ǩ */
	FormMP3->FontBlack.FontCode = FC_ST_16;
	FormMP3->FontBlack.BackColor = CL_MASK;		/* ͸��ɫ */
	FormMP3->FontBlack.FrontColor = CL_BLACK;
	FormMP3->FontBlack.Space = 0;

	/* ����2 ���ڱ仯������ */
	FormMP3->FontBlue.FontCode = FC_ST_16;
	FormMP3->FontBlue.BackColor = CL_BTN_FACE;
	FormMP3->FontBlue.FrontColor = CL_BLUE;
	FormMP3->FontBlue.Space = 0;

	/* ��ť���� */
	FormMP3->FontBtn.FontCode = FC_ST_16;
	FormMP3->FontBtn.BackColor = CL_MASK;		/* ͸������ */
	FormMP3->FontBtn.FrontColor = CL_BLACK;
	FormMP3->FontBtn.Space = 0;

	/* ����� */
	FormMP3->Box1.Left = BOX1_X;
	FormMP3->Box1.Top = BOX1_Y;
	FormMP3->Box1.Height = BOX1_H;
	FormMP3->Box1.Width = BOX1_W;
	FormMP3->Box1.pCaption = BOX1_TEXT;
	FormMP3->Box1.Font = &FormMP3->FontBox;

	/* ��̬��ǩ */
	FormMP3->Label1.Left = LABEL1_X;
	FormMP3->Label1.Top = LABEL1_Y;
	FormMP3->Label1.MaxLen = 0;
	FormMP3->Label1.pCaption = LABEL1_TEXT;
	FormMP3->Label1.Font = &FormMP3->FontBlack;

	FormMP3->Label3.Left = LABEL3_X;
	FormMP3->Label3.Top = LABEL3_Y;
	FormMP3->Label3.MaxLen = 0;
	FormMP3->Label3.pCaption = LABEL3_TEXT;
	FormMP3->Label3.Font = &FormMP3->FontBlack;

	FormMP3->Label5.Left = LABEL5_X;
	FormMP3->Label5.Top = LABEL5_Y;
	FormMP3->Label5.MaxLen = 0;
	FormMP3->Label5.pCaption = LABEL5_TEXT;
	FormMP3->Label5.Font = &FormMP3->FontBlack;

	FormMP3->Label7.Left = LABEL7_X;
	FormMP3->Label7.Top = LABEL7_Y;
	FormMP3->Label7.MaxLen = 0;
	FormMP3->Label7.pCaption = LABEL7_TEXT;
	FormMP3->Label7.Font = &FormMP3->FontBlack;

	/* ��̬��ǩ */
	FormMP3->Label2.Left = LABEL2_X;
	FormMP3->Label2.Top = LABEL2_Y;
	FormMP3->Label2.MaxLen = 0;
	FormMP3->Label2.pCaption = LABEL2_TEXT;
	FormMP3->Label2.Font = &FormMP3->FontBlue;

	FormMP3->Label4.Left = LABEL4_X;
	FormMP3->Label4.Top = LABEL4_Y;
	FormMP3->Label4.MaxLen = 0;
	FormMP3->Label4.pCaption = LABEL4_TEXT;
	FormMP3->Label4.Font = &FormMP3->FontBlue;

	FormMP3->Label6.Left = LABEL6_X;
	FormMP3->Label6.Top = LABEL6_Y;
	FormMP3->Label6.MaxLen = 0;
	FormMP3->Label6.pCaption = LABEL6_TEXT;
	FormMP3->Label6.Font = &FormMP3->FontBlue;

	FormMP3->Label8.Left = LABEL8_X;
	FormMP3->Label8.Top = LABEL8_Y;
	FormMP3->Label8.MaxLen = 0;
	FormMP3->Label8.pCaption = LABEL8_TEXT;
	FormMP3->Label8.Font = &FormMP3->FontBlue;

	/* ��ť */
	FormMP3->BtnRet.Left = BTN_RET_X;
	FormMP3->BtnRet.Top = BTN_RET_Y;
	FormMP3->BtnRet.Height = BTN_RET_H;
	FormMP3->BtnRet.Width = BTN_RET_W;
	FormMP3->BtnRet.pCaption = BTN_RET_TEXT;
	FormMP3->BtnRet.Font = &FormMP3->FontBtn;
	FormMP3->BtnRet.Focus = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: DispFormMP3
*	����˵��: ��ʾ���еľ�̬�ؼ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispFormMP3(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* ����� */
	LCD_DrawGroupBox(&FormMP3->Box1);

	/* ��̬��ǩ */
	LCD_DrawLabel(&FormMP3->Label1);
	LCD_DrawLabel(&FormMP3->Label3);
	LCD_DrawLabel(&FormMP3->Label5);
	LCD_DrawLabel(&FormMP3->Label7);

	/* ��̬��ǩ */
	LCD_DrawLabel(&FormMP3->Label2);
	LCD_DrawLabel(&FormMP3->Label4);
	LCD_DrawLabel(&FormMP3->Label6);
	LCD_DrawLabel(&FormMP3->Label8);

	/* ��ť */
	LCD_DrawButton(&FormMP3->BtnRet);
}


/*
*********************************************************************************************************
*	�� �� ��: MP3HardInit
*	����˵��: ����MP3������ص�Ӳ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MP3HardInit(void)
{
	/* ����VS1053Ӳ�� */
	{
		#if 0
		/* �ȴ�оƬ�ڲ�������� */
		if (vs1053_WaitTimeOut())
		{
			/* ���û�в�VS1053Bģ�飬DREQ���߽����ص͵�ƽ������һ���쳣��� */
			FormMP3->Label2.pCaption = "û�м�⵽MP3ģ��Ӳ��";
			LCD_DrawLabel(&FormMP3->Label2);		/* ��ʾоƬ�ͺ� */

			return;
		}
		#endif
		
		vs1053_Init();
		vs1053_SoftReset();

		/* ��ӡMP3����оƬ�ͺ� */
		{
			char *pModel;

			switch (vs1053_ReadChipID())
			{
				case VS1001:
					pModel = "VS1001";
					break;

				case VS1011:
					pModel = "VS1011";
					break;

				case VS1002:
					pModel = "VS1002";
					break;

				case VS1003:
					pModel = "VS1003";
					break;

				case VS1053:
					pModel = "VS1053";
					break;

				case VS1033:
					pModel = "VS1033";
					break;

				case VS1103:
					pModel = "VS1103";
					break;

				default:
					pModel = "unknow";
					break;
			}
			FormMP3->Label2.pCaption = pModel;
			LCD_DrawLabel(&FormMP3->Label2);		/* ��ʾоƬ�ͺ� */
		}

		g_tMP3.ucVolume = 200; 			/* ȱʡ����,Խ������ԽС */
		vs1053_SetVolume(g_tMP3.ucVolume);

		vs1053_SetBASS(0, 0, 0, 0);		/* ��Ƶ�͵�������ǿ */
	}

	/* ����WM8978��Ƶͨ��, ���Խ�VS1053B�������Ƶ����WM8978��Line��������� ���������ϵ����������� */
	{
		/* bsp.c ���Ѿ���ʼ��I2C���� */
		wm8978_Init();		/* ��λWM8978����λ״̬ */

		wm8978_SetSpkVolume(0);	/* ��ʱ���������� */

		/* ����WM8978оƬ������ΪLINE IN�����Ϊ������������ */
		wm8978_CfgAudioPath(LINE_ON, EAR_LEFT_ON | EAR_RIGHT_ON | SPK_ON);
		/* ���ڷ���������������ͬ���� */
		wm8978_SetEarVolume(30);	/* ���ö������������63 */
		wm8978_SetSpkVolume(60);	/* �������������������63 */
		wm8978_SetLineGain(6);		/* ����Line�������棬 0-7 */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: FillSongList
*	����˵��: �����������б�����NAND Flash��Ŀ¼�µĸ��������10��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void FillSongList(void)
{
	/* ����Fatfs�õ���ȫ�ֱ��� */
	FRESULT result;
	FILINFO FileInf;
	DIR DirInf;
	char path[128];
	char lfname[256];

	/* �򿪸���Ŀ¼ */
	sprintf(path, "%s", MP3_PATH);
	result = f_opendir(&DirInf, path); 	/* path���Դ��̷������һ���ַ�������/ */
	if (result != FR_OK)
	{
		printf("Open Root Directory Error (%d)\r\n", result);
	}

	/* ��ȡ��ǰ�ļ����µ��ļ���Ŀ¼ */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;	
	
	g_tMP3.ListIndex = 0;
	g_tMP3.ListCount = 0;	/* �������� */
	for(;;)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* ��ȡĿ¼��������Զ����� */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')	/* ��ʾĿ¼ */
		{
			continue;
		}

		if (FileInf.fattrib != AM_DIR)
		{
			uint8_t Len;

			Len = strlen(FileInf.fname);
			if (Len >= 5)
			{
				if (memcmp(&FileInf.fname[Len - 3], "MP3", 3) == 0)
				{
					/* ����MP3�ļ����������б� */
					strcpy(FormMP3->List[g_tMP3.ListCount].FileName, FileInf.fname);
					FormMP3->List[g_tMP3.ListCount].FileSize = FileInf.fsize;
					g_tMP3.ListCount++;		/* �������� */

					/* ���MP3�ļ������������˳� */
					if (g_tMP3.ListCount > SONG_LIST_MAX)
					{
						break;
					}

				}
			}
		}
	}

	if (g_tMP3.ListCount == 0)
	{
		printf("û���ڸ�Ŀ¼���ҵ� MP3 �ļ�\r\n");
	}
}

/*
*********************************************************************************************************
*	�� �� ��: Mp3Pro
*	����˵��: MP3�ļ����ţ���������whileѭ���е���. ÿ����VS105B����32�ֽڡ�
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ��������; 1 ��ʾ�ļ��������,������ݴ��л�����һ�׸���
*********************************************************************************************************
*/
static uint8_t Mp3Pro(void)
{
	uint32_t bw,i;
	char buf[32];

	/* ���VS1003���У���д���µ����� */
	if (vs1053_ReqNewData())
	{
		f_read(&FormMP3->file, &buf, 32, &bw);
		if (bw <= 0)
		{
			return 1;
		}

		/* ������� */
		g_tMP3.uiProgress += bw;

		for (i = 0; i < bw; i++)
		{
			vs1053_WriteData(buf[i]);
		}
	}
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: Mp3DispStatus
*	����˵��: ��ʾ��ǰ״̬
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void Mp3DispStatus(void)
{
	char buf[5];

	/* ��ʾ���� */
	sprintf(buf, "%3d ", g_tMP3.ucVolume);
	FormMP3->Label4.pCaption = buf;
	LCD_DrawLabel(&FormMP3->Label4);

	/* ��ʾ���� */
	sprintf(buf, "%3d%%", g_tMP3.uiProgress * 100 / FormMP3->List[g_tMP3.ListIndex].FileSize);
	FormMP3->Label6.pCaption = buf;
	LCD_DrawLabel(&FormMP3->Label6);
}

/*
*********************************************************************************************************
*	�� �� ��: ViewDir
*	����˵��: ��ʾ��Ŀ¼�µ��ļ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ViewDir(char *_path)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, _path); /* 1: ��ʾ�̷� */
	if (result != FR_OK)
	{
		printf("�򿪸�Ŀ¼ʧ�� (%d)\r\n", result);
		return;
	}

	printf("\r\n��ǰĿ¼��%s\r\n", _path);

	/* ��ȡ��ǰ�ļ����µ��ļ���Ŀ¼ */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;

	printf("����        |  �ļ���С | ���ļ��� | ���ļ���\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* ��ȡĿ¼��������Զ����� */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* �ж����ļ�������Ŀ¼ */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)Ŀ¼  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)�ļ�  ", FileInf.fattrib);
		}

		/* ��ӡ�ļ���С, ���4G */
		printf(" %10d", FileInf.fsize);

		printf("  %s |", FileInf.fname);	/* ���ļ��� */

		printf("  %s\r\n", (char *)FileInf.lfname);	/* ���ļ��� */
	}

	f_closedir(&DirInf);	/*���رմ򿪵�Ŀ¼ */
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/

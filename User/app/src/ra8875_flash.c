/*
*********************************************************************************************************
*
*	模块名称 : RA8875屏外挂的串行Flash模块
*	文件名称 : ra8875_flash.c
*	版    本 : V1.0
*	说    明 : 读写RA8875屏上的字库芯片和图库芯片。RA8875本身不支持写串行Flash ，只支持DMA方式读数据。
*			   通过增加额外的电子开关将串行Flash切换到STM32就可以改写数据。仅针对支持该功能的RA8875屏。
*			   安富莱 RA8875 7寸屏(D017-2)支持在线刷新字库和图库芯片。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2013-02-01 armfly  首发
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "ra8875_flash.h"
#include "ff.h"			/* FatFS文件系统模块*/

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack;	/* 静态的文字 */
	FONT_T FontBlue;	/* 变化的文字字体 */
	FONT_T FontBtn;		/* 按钮的字体 */
	FONT_T FontBox;		/* 分组框标题字体 */

	GROUP_T Box1;
	GROUP_T Box2;

	LABEL_T Label1;	LABEL_T Label2;
	LABEL_T Label3; LABEL_T Label4;
	LABEL_T Label5; LABEL_T Label6;
	LABEL_T Label7; LABEL_T Label8;

	BUTTON_T BtnRet;

	BUTTON_T Btn1;		/* 读字库芯片 */
	BUTTON_T Btn2;		/* 写字库芯片 */
	BUTTON_T Btn3;		/* 读图片芯片 */
	BUTTON_T Btn4;		/* 读字库芯片 */
	BUTTON_T Btn5;		/* 进入休眠，测试功耗 */

	BUTTON_T Btn6;		/* 擦除NORFlash */
	BUTTON_T Btn7;		/* 写NORFlash */
}FormW25_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	100
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"RA8875字库和图库芯片"

/* 第1个框内的按钮 */
#define BTN1_H	32
#define BTN1_W	120
#define	BTN1_X	(BOX1_X + 10)
#define	BTN1_Y	(BOX1_Y + 20)
#define	BTN1_TEXT	"读字库芯片"

#define BTN2_H	BTN1_H
#define BTN2_W	BTN1_W
#define	BTN2_X	(BTN1_X +  BTN1_W + 10)
#define	BTN2_Y	BTN1_Y
#define	BTN2_TEXT	"写字库芯片"

#define BTN3_H	BTN1_H
#define BTN3_W	BTN1_W
#define	BTN3_X	BTN1_X
#define	BTN3_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN3_TEXT	"读图库芯片"

#define BTN4_H	BTN1_H
#define BTN4_W	BTN1_W
#define	BTN4_X	(BTN1_X +  BTN1_W + 10)
#define	BTN4_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN4_TEXT	"写图片芯片"

#define BTN5_H	BTN1_H
#define BTN5_W	BTN1_W
#define	BTN5_X	(BTN1_X +  2 * (BTN1_W + 10))
#define	BTN5_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN5_TEXT	"关闭LCD"


/* 第2个框 */
#define BOX2_X	BOX1_X
#define BOX2_Y	(BOX1_Y + BOX1_H + 10)
#define BOX2_H	60
#define BOX2_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX2_TEXT	"NOR Flash芯片"

#define BTN6_H	32
#define BTN6_W	120
#define	BTN6_X	(BOX2_X + 10)
#define	BTN6_Y	(BOX2_Y + 20)
#define	BTN6_TEXT	"擦除NOR Flash"

#define BTN7_H	BTN6_H
#define BTN7_W	BTN6_W
#define	BTN7_X	(BTN6_X +  1 * (BTN1_W + 10))
#define	BTN7_Y	BTN6_Y
#define	BTN7_TEXT	"写NOR Flash"

/* 按钮 */
/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H	32
#define BTN_RET_W	80
#define	BTN_RET_X	(g_LcdWidth - BTN_RET_W - 8)
#define	BTN_RET_Y	(g_LcdHeight - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"返回"


#define LABEL1_X  	(BOX2_X + 6)
#define LABEL1_Y	(BOX2_Y + BOX2_H + 10)
#define LABEL1_TEXT	"地址 : "

	#define LABEL2_X  	(LABEL1_X + 64)
	#define LABEL2_Y	LABEL1_Y
	#define LABEL2_TEXT	"0"

#define LABEL3_X  	(LABEL1_X)
#define LABEL3_Y	(LABEL1_Y + 20)
#define LABEL3_TEXT	"耗时 : "

	#define LABEL4_X  	(LABEL3_X + 64)
	#define LABEL4_Y	(LABEL3_Y)
	#define LABEL4_TEXT	"0"

#define LABEL5_X  	(LABEL1_X)
#define LABEL5_Y	(LABEL1_Y + 20 * 2)
#define LABEL5_TEXT	"状态 : "

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
*	函 数 名: RA8875FlashMain
*	功能说明: 读写RA8875外挂的串行Flash
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875FlashMain(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	FormW25_T form;

	FormW25= &form;

	InitFormW25();

	DispFormW25();

	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
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

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (LCD_ButtonTouchRelease(&FormW25->BtnRet, tpX, tpY))
					{
						fQuit = 1;	/* 返回 */
					}

					if (g_ChipID == IC_8875)
					{
						if (LCD_ButtonTouchRelease(&FormW25->Btn1, tpX, tpY))
						{
							ReadRA8875Flash(FONT_CHIP);	/* 读字库芯片 */
						}
						if (LCD_ButtonTouchRelease(&FormW25->Btn2, tpX, tpY))
						{
							WriteRA8875Flash(FONT_CHIP);	/* 写字库芯片 */
						}
						 if (LCD_ButtonTouchRelease(&FormW25->Btn3, tpX, tpY))
						{
							ReadRA8875Flash(BMP_CHIP);	/* 读图库芯片 */
						}
						if (LCD_ButtonTouchRelease(&FormW25->Btn4, tpX, tpY))
						{
							WriteRA8875Flash(BMP_CHIP);	/* 写图库芯片 */
						}
						if (LCD_ButtonTouchRelease(&FormW25->Btn5, tpX, tpY))
						{
							RA8875_Sleep();	/* 关闭显示，进入休眠 */
							while (1);
						}						
					}

					if (LCD_ButtonTouchRelease(&FormW25->Btn6, tpX, tpY))
					{
						EraseNorFlash();		/* 擦除NORFlash */
					}
					if (LCD_ButtonTouchRelease(&FormW25->Btn7, tpX, tpY))
					{
						WriteNorFlash();		/* 写NORFlash */
					}
					
					/* 清触摸缓冲区，避免长时间等待后，用户错误点击按钮，又重新写一遍 */
					TOUCH_CelarFIFO();				
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
				case KEY_DOWN_K1:		/* K1键 */
					WriteRA8875Flash(FONT_CHIP);	/* 写字库芯片 */
					break;

				case KEY_DOWN_K2:		/* K2键按下 */
					WriteRA8875Flash(BMP_CHIP);	/* 写图库芯片 */
					break;

				case KEY_DOWN_K3:		/* K3键按下 */
					break;

				case JOY_DOWN_U:		/* 摇杆UP键按下 */
					break;

				case JOY_DOWN_D:		/* 摇杆DOWN键按下 */
					break;

				case JOY_DOWN_L:		/* 摇杆LEFT键按下 */
					break;

				case JOY_DOWN_R:		/* 摇杆RIGHT键按下 */
					fQuit = 1;	/* 返回 */
					break;

				case JOY_DOWN_OK:		/* 摇杆OK键按下 */
					break;

				default:
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: InitFormW25
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormW25(void)
{
	/* 分组框标题字体 */
	FormW25->FontBox.FontCode = FC_ST_16;
	FormW25->FontBox.BackColor = CL_BTN_FACE;	/* 和背景色相同 */
	FormW25->FontBox.FrontColor = CL_BLACK;
	FormW25->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormW25->FontBlack.FontCode = FC_ST_16;
	FormW25->FontBlack.BackColor = CL_MASK;		/* 透明色 */
	FormW25->FontBlack.FrontColor = CL_BLACK;
	FormW25->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormW25->FontBlue.FontCode = FC_ST_16;
	FormW25->FontBlue.BackColor = CL_BTN_FACE;
	FormW25->FontBlue.FrontColor = CL_BLUE;
	FormW25->FontBlue.Space = 0;

	/* 按钮字体 */
	FormW25->FontBtn.FontCode = FC_ST_16;
	FormW25->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
	FormW25->FontBtn.FrontColor = CL_BLACK;
	FormW25->FontBtn.Space = 0;

	/* 分组框 */
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

	/* 静态标签 */
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

	/* 动态标签 */
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

	/* 按钮 */
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
*	函 数 名: DispFormW25
*	功能说明: 显示所有的静态控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFormW25(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
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

	/* 按钮 */
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
*	函 数 名: ReadRA8875Flash
*	功能说明: 读RA8875 Flash内容
*	形    参: _ucChipType : 芯片类型 FONT_CHIP 或 BMP_CHIP
*	返 回 值: 无
*********************************************************************************************************
*/
void ReadRA8875Flash(uint8_t _ucChipType)
{
	uint32_t i;
	uint8_t buf[512];
	char cDispBuf[32];
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char path[32];
	uint8_t ucVolume = FS_NAND;
	char volume_str[5];
	int32_t iTime1, iTime2;

	/* 配置RA8875串行Flash相关的硬件 */
	bsp_InitRA8875Flash();
	w25_CtrlByMCU();
	w25_SelectChip(_ucChipType);

	/* 首先需要判断RA8875屏是否支持串行Flash的读写 */
	{
		/* 识别Flash是否可读写 */
		if ((g_tW25.ChipID != W25Q64BV) && (g_tW25.ChipID != W25Q128))
		{
			if ((_ucChipType == FONT_CHIP) && (g_tW25.ChipID == 0))
			{
				/* 读最后数据 */
				w25_ReadBuffer(buf, 0x1FFFF0, 5);

				if (memcmp(buf, "GT20K", 5) == 0)
				{
					/* 安装的集通字库芯片(可以读取) */
				}
				else
				{
					FormW25->Label6.pCaption = "显示模块硬件不支持读字库芯片";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit2;
				}
			}
			else
			{
				if (_ucChipType == FONT_CHIP)
				{
					FormW25->Label6.pCaption = "显示模块硬件不支持读字库芯片";
				}
				else
				{
					FormW25->Label6.pCaption = "显示模块硬件不支持读图库芯片";
				}
				LCD_DrawLabel(&FormW25->Label6);

				goto quit2;
			}
		}
	}

	/* 文件系统操作 */
	{
		/* 挂载文件系统 */
		sprintf(volume_str, "%d:", ucVolume);
		result = f_mount(&fs, volume_str, 0);			/* Mount a logical drive */
		if (result != FR_OK)
		{
			printf("挂载文件系统失败 (%d)\r\n", result);

			FormW25->Label6.pCaption = "挂载文件系统失败                  ";
			LCD_DrawLabel(&FormW25->Label6);

			goto quit1;
		}

		/* 打开根文件夹 */
		sprintf(path, "%d:/RA8875", ucVolume);
		result = f_opendir(&DirInf, path);
		if (result != FR_OK)
		{
			printf("打开根目录失败 (%d)\r\n", result);

			FormW25->Label6.pCaption = "打开根目录失败             ";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}

		/* 打开文件 */
		if (_ucChipType == FONT_CHIP)
		{
			sprintf(path, "%d:/RA8875/FontRead.bin", ucVolume);

			FormW25->Label6.pCaption = "正在读字库芯片, 保存到文件FontRead.bin";
			LCD_DrawLabel(&FormW25->Label6);
		}
		else
		{
			sprintf(path, "%d:/RA8875/BmpRead.bin", ucVolume);

			FormW25->Label6.pCaption = "正在读图库芯片, 保存到文件BmpRead.bin";
			LCD_DrawLabel(&FormW25->Label6);
		}
		result = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);
	}

	iTime1 = bsp_GetRunTime();
	/* 该Label用于显示进度 */
	FormW25->Label2.pCaption = cDispBuf;
	FormW25->Label4.pCaption = cDispBuf;

	for (i = 0; i < g_tW25.TotalSize / 512; i++)
	{
		sprintf(cDispBuf, "0x%06X (%%%02d)  ", i * 512, ((i + 1) * 100) /(g_tW25.TotalSize / 512));
		LCD_DrawLabel(&FormW25->Label2);

		iTime2 = bsp_GetRunTime() - iTime1;
		sprintf(cDispBuf, "%d 秒      ", iTime2 / 1000);
		LCD_DrawLabel(&FormW25->Label4);

		w25_ReadBuffer(buf, i * 512, 512);

		/* 写256字节数据 */
		result = f_write(&file, buf, 512, &bw);
		if (result != FR_OK)
		{
			printf("%s 文件写入失败\r\n", path);

			FormW25->Label6.pCaption = "文件写入失败";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}
	}

	FormW25->Label6.pCaption = "读成功                                         ";
	LCD_DrawLabel(&FormW25->Label6);

quit1:
	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, volume_str, 0);

quit2:
	w25_CtrlByRA8875();
}

/*
*********************************************************************************************************
*	函 数 名: WriteRA8875Flash
*	功能说明: 写RA8875 Flash
*	形    参: _ucChipType : 芯片类型，FONT_CHIP 或 BMP_CHIP
*	返 回 值: 无
*********************************************************************************************************
*/
void WriteRA8875Flash(uint8_t _ucChipType)
{
	uint32_t i;
	uint8_t buf[512], buf_read[512];
	char cDispBuf[32];
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
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

	/* 配置RA8875串行Flash相关的硬件 */
	bsp_InitRA8875Flash();
	w25_CtrlByMCU();
	w25_SelectChip(_ucChipType);

	/* 首先需要判断RA8875屏是否支持串行Flash的读写 */
	{
		/* 识别Flash是否可读写 */
		if ((g_tW25.ChipID != W25Q64BV) && (g_tW25.ChipID != W25Q128))
		{
			if ((_ucChipType == FONT_CHIP) && (g_tW25.ChipID == 0))
			{
				/* 读最后数据 */
				w25_ReadBuffer(buf, 0x1FFFF0, 5);

				if (memcmp(buf, "GT20K", 5) == 0)
				{
					/* 安装的集通字库芯片(可以读取) */
					FormW25->Label6.pCaption = "显示模块的字库芯片是只读的           ";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit1;
				}
				else
				{
					FormW25->Label6.pCaption = "显示模块硬件不支持写字库芯片         ";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit1;
				}
			}
			else
			{
				if (_ucChipType == FONT_CHIP)
				{
					FormW25->Label6.pCaption = "显示模块硬件不支持写字库芯片";
				}
				else
				{
					FormW25->Label6.pCaption = "显示模块硬件不支持写图库芯片";
				}
				LCD_DrawLabel(&FormW25->Label6);

				goto quit1;
			}
		}
	}

	/* 文件系统操作 */
	{
		/* 挂载文件系统 */
		sprintf(volume_str, "%d:", ucVolume);
		result = f_mount(&fs, volume_str, 0);			/* Mount a logical drive */
		if (result != FR_OK)
		{
			printf("挂载文件系统失败 (%d)\r\n", result);

			FormW25->Label6.pCaption = "挂载文件系统失败";
			LCD_DrawLabel(&FormW25->Label6);

			goto quit1;
		}

		/* 打开文件 */
		if (_ucChipType == FONT_CHIP)
		{
			sprintf(path, "%d:/RA8875/FontWrite.bin", ucVolume);

			/* 打开文件 */
			result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
			if (result !=  FR_OK)
			{
				FormW25->Label6.pCaption = "没有找到文件FontWrite.bin";
				LCD_DrawLabel(&FormW25->Label6);

				goto quit1;
			}

			/* 获得文件大小 */
			{
				FILINFO fno;

				fno.lfname = 0;		/* 必须设置为0，因为长文件名使能后，f_stat函数内部会写这个缓冲区 */
				fno.lfsize = 0;				

				f_stat(path, &fno);
				WriteSize = fno.fsize;

				/* 文件大小不能大于串行Flash的容量 8MB */
				if (WriteSize > g_tW25.TotalSize)
				{
					FormW25->Label6.pCaption = "FontWrite.bin文件长度过大";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit1;
				}
			}

			FormW25->Label6.pCaption = "正在写字库芯片,读文件FontWrite.bin";
			LCD_DrawLabel(&FormW25->Label6);
		}
		else
		{
			sprintf(path, "%d:/RA8875/BmpWrite.bin", ucVolume);

			result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
			if (result !=  FR_OK)
			{
				FormW25->Label6.pCaption = "没有找到文件BmpWrite.bin";
				LCD_DrawLabel(&FormW25->Label6);

				goto quit1;
			}

			/* 获得文件大小。 注意: 长文件名 */
			{
				FILINFO fno;
				
				fno.lfname = 0;		/* 必须设置为0，因为长文件名使能后，f_stat函数内部会写这个缓冲区 */
				fno.lfsize = 0;

				f_stat(path, &fno);
				WriteSize = fno.fsize;

				/* 文件大小不能大于串行Flash的容量 8MB */
				if (WriteSize > g_tW25.TotalSize)
				{
					FormW25->Label6.pCaption = "BmpWrite.bin文件长度过大";
					LCD_DrawLabel(&FormW25->Label6);

					goto quit1;
				}
			}

			FormW25->Label6.pCaption = "正在写图库芯片, 文件BmpWrite.bin";
			LCD_DrawLabel(&FormW25->Label6);

			/* 对于铁框屏，一片 W25Q128，前面2MB是字库。后面14MB是图片 */
			if (g_tW25.ChipID == W25Q128)
			{
				OffsetAddr = PIC_OFFSET;	/* 前面2MB分配给字库 */
			}
		}
	}

	iTime1 = bsp_GetRunTime();
	/* 该Label用于显示进度 */
	FormW25->Label2.pCaption = cDispBuf;
	FormW25->Label4.pCaption = cDispBuf;

	for (i = 0; i < WriteSize / 512; i++)
	{
		if ((i % 16) == 0)
		{
			sprintf(cDispBuf, "0x%06X / 0x%06X(%%%02d)  ", i * 512, WriteSize, ((i + 16) * 100) /(WriteSize / 512));
			LCD_DrawLabel(&FormW25->Label2);

			iTime2 = bsp_GetRunTime() - iTime1;
			sprintf(cDispBuf, "%d 秒", iTime2 / 1000);
			LCD_DrawLabel(&FormW25->Label4);
		}

		result = f_read(&file, &buf, 512, &bw);
		if (result != FR_OK)
		{
			FormW25->Label6.pCaption = "文件读取失败";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}

		if (((i * 512) % g_tW25.PageSize) == 0)
		{
			w25_EraseSector(OffsetAddr + i * 512);
		}
		w25_WritePage(buf, OffsetAddr + i * 512, 512);		/* 写入串行Flash */
		w25_ReadBuffer(buf_read, OffsetAddr + i * 512, 512);	/* 读回进行比较 */
		if (memcmp(buf, buf_read, 512) != 0)
		{
			FormW25->Label6.pCaption = "写串行Flash出错!";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}
	}

	if (_ucChipType == FONT_CHIP)
	{
		FormW25->Label6.pCaption = "写字库芯片成功";
	}
	else
	{
		FormW25->Label6.pCaption = "写图库芯片成功";
	}
	LCD_DrawLabel(&FormW25->Label6);

quit1:
	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, volume_str, 0);

	w25_CtrlByRA8875();
}

/*
*********************************************************************************************************
*	函 数 名: EraseNorFlash
*	功能说明: 擦除NOR Flash
*	形    参:
*	返 回 值: 无
*********************************************************************************************************
*/
static void EraseNorFlash(void)
{
	uint32_t i;
	char cDispBuf[32];
	int32_t iTime1, iTime2;

	FormW25->Label6.pCaption = "正在擦除NOR Flash芯片";
	LCD_DrawLabel(&FormW25->Label6);

	iTime1 = bsp_GetRunTime();

	/* 该Label用于显示进度 */
	FormW25->Label2.pCaption = cDispBuf;
	FormW25->Label4.pCaption = cDispBuf;

#if 0	/* 执行整片擦除 (58秒), 和按BLOCK擦除时间差不多。 */
	NOR_StartEraseChip();	/* 开始擦除， 此过程很长,100多秒 */

	for (;;)
	{
		if (NOR_CheckStatus() == NOR_SUCCESS)
		{
			break;
		}

		iTime2 = bsp_GetRunTime() - iTime1;
		sprintf(cDispBuf, "%d 秒           ", iTime2 / 1000);
		LCD_DrawLabel(&FormW25->Label4);

		sprintf(cDispBuf, "%%%d",  ((iTime2 / 1000) * 100) / 59);
		LCD_DrawLabel(&FormW25->Label2);
	}

	FormW25->Label6.pCaption = "擦除NOR Flash成功";
	LCD_DrawLabel(&FormW25->Label6);
#else	/* 按照 BLOCK 进行擦除 (57秒) */
	for (i = 0; i < NOR_SECTOR_COUNT; i++)
	{
		sprintf(cDispBuf, "0x%06X (%%%02d)  ", i * NOR_SECTOR_SIZE, ((i + 1) * 100) / NOR_SECTOR_COUNT);
		LCD_DrawLabel(&FormW25->Label2);

		iTime2 = bsp_GetRunTime() - iTime1;
		sprintf(cDispBuf, "%d 秒      ", iTime2 / 1000);
		LCD_DrawLabel(&FormW25->Label4);

		NOR_EraseSector(i * NOR_SECTOR_SIZE);
	}

	FormW25->Label6.pCaption = "擦除NOR Flash成功";
	LCD_DrawLabel(&FormW25->Label6);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: WriteNorFlash
*	功能说明: 将文件内容写入NOR Flash.  (执行时间 59秒 @ 5MB)
*	形    参:
*	返 回 值: 无
*********************************************************************************************************
*/
static void WriteNorFlash(void)
{
	uint32_t i;
	uint8_t buf[512];
	char cDispBuf[32];

	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	uint32_t bw;
	char path[32];
	uint8_t ucVolume = FS_NAND;
	char volume_str[5];
	int32_t iTime1, iTime2;

	/* 文件系统操作 */
	{
		/* 挂载文件系统 */
		sprintf(volume_str, "%d:", ucVolume);
		result = f_mount(&fs, volume_str, 0);			/* Mount a logical drive */
		if (result != FR_OK)
		{
			printf("挂载文件系统失败 (%d)\r\n", result);

			FormW25->Label6.pCaption = "挂载文件系统失败";
			LCD_DrawLabel(&FormW25->Label6);

			goto quit1;
		}

		/* 打开文件 */
		sprintf(path, "%d:/NorFlash/NorFlash.bin", ucVolume);

		result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
		if (result !=  FR_OK)
		{
			FormW25->Label6.pCaption = "没有找到文件NorFlash.bin";
			LCD_DrawLabel(&FormW25->Label6);

			goto quit1;
		}

		FormW25->Label6.pCaption = "正在写NOR Flash芯片,读文件NorFlash.bin";
		LCD_DrawLabel(&FormW25->Label6);
	}

	iTime1 = bsp_GetRunTime();
	/* 该Label用于显示进度 */
	FormW25->Label2.pCaption = cDispBuf;
	FormW25->Label4.pCaption = cDispBuf;

	/* NOR Flash 支持页内(64字节)连续编程,但是不能跨页编程 */
	for (i = 0; i < file.fsize / 64; i++)
	{
		if ((i % 10) == 0)
		{
			sprintf(cDispBuf, "0x%06X (%%%02d)  ", i * 64, ((i + 1) * 100) /(file.fsize / 64));
			LCD_DrawLabel(&FormW25->Label2);

			iTime2 = bsp_GetRunTime() - iTime1;
			sprintf(cDispBuf, "%d 秒      ", iTime2 / 1000);
			LCD_DrawLabel(&FormW25->Label4);
		}
		result = f_read(&file, &buf, 64, &bw);
		if (result != FR_OK)
		{
			printf("%s 文件读取失败\r\n", path);

			FormW25->Label6.pCaption = "文件读取失败";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}
		if (NOR_WriteInPage((uint16_t *)&buf, i * 64, 64 / 2) != NOR_SUCCESS)
		{
			FormW25->Label6.pCaption = "NOR编程失败";
			LCD_DrawLabel(&FormW25->Label6);
			goto quit1;
		}
	}

	sprintf(cDispBuf, "0x%06X (%%%02d)  ", i * 64, 100);
	LCD_DrawLabel(&FormW25->Label2);

	FormW25->Label6.pCaption = "写成功";
	LCD_DrawLabel(&FormW25->Label6);

quit1:
	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, volume_str, 0);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

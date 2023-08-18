/*
*********************************************************************************************************
*
*	模块名称 : 步进电机测试界面
*	文件名称 : form_step_moto.c
*	版    本 : V1.0
*	说    明 : 5线减速步进电机 28BYJ48。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2015-07-19 armfly  
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_step_moto.h"

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack;	/* 黑色 */
	FONT_T FontBlue;	/* 蓝色 */
	FONT_T FontBtn;		/* 按钮的字体 */
	FONT_T FontBox;		/* 分组框标题字体 */

	GROUP_T Box1;

	BUTTON_T BtnRet;

	BUTTON_T Btn1;
	BUTTON_T Btn2;
	BUTTON_T Btn3;
	BUTTON_T Btn4;
	BUTTON_T Btn5;
	BUTTON_T Btn6;
	BUTTON_T Btn7;
	BUTTON_T Btn8;
	BUTTON_T Btn9;
	BUTTON_T Btn10;
	BUTTON_T Btn11;
	BUTTON_T Btn12;
	BUTTON_T Btn13;
	BUTTON_T Btn14;
	BUTTON_T Btn15;
	BUTTON_T Btn16;	

	LABEL_T Label1;
	LABEL_T Label2;
	LABEL_T Label3;
	LABEL_T Label4;
	LABEL_T Label5;
	LABEL_T Label6;

	uint16_t DacValue;	/* DAC数据 */
	int32_t Voltage;	/* 输出电压 */
}FormMoto_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	(g_LcdHeight - 53)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_T	"步进电机测试（28BYJ48）"

#define BTN1_H	32
#define BTN1_W	105

/* 第1行按钮坐标 */
#define	BTN1_X	(BOX1_X + 10)
#define	BTN1_Y	(BOX1_Y + 20)
#define	BTN1_T	"正转"

	#define BTN2_H	BTN1_H
	#define BTN2_W	BTN1_W
	#define	BTN2_X	(BTN1_X +  BTN1_W + 10)
	#define	BTN2_Y	BTN1_Y
	#define	BTN2_T	"反转"

		#define BTN3_H	BTN1_H
		#define BTN3_W	BTN1_W
		#define	BTN3_X	(BTN1_X + 2 * (BTN1_W + 10))
		#define	BTN3_Y	BTN1_Y
		#define	BTN3_T	"暂停(抱死)"
		
			#define BTN16_H	BTN1_H
			#define BTN16_W	BTN1_W
			#define	BTN16_X	(BTN1_X + 3 * (BTN1_W + 10))
			#define	BTN16_Y	BTN1_Y
			#define	BTN16_T	"停机"	

/* 第2行按钮坐标 */
#define BTN4_H	BTN1_H
#define BTN4_W	BTN1_W
#define	BTN4_X	BTN1_X
#define	BTN4_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN4_T	"步进频率+1"

	#define BTN5_H	BTN1_H
	#define BTN5_W	BTN1_W
	#define	BTN5_X	(BTN1_X +  1 * (BTN1_W + 10))
	#define	BTN5_Y	BTN4_Y
	#define	BTN5_T	"步进频率-1"

		#define BTN6_H	BTN1_H
		#define BTN6_W	BTN1_W
		#define	BTN6_X	(BTN1_X +  2 * (BTN1_W + 10))
		#define	BTN6_Y	BTN4_Y
		#define	BTN6_T	"步进频率+10"

			#define BTN7_H	BTN1_H
			#define BTN7_W	BTN1_W
			#define	BTN7_X	(BTN1_X +  3 * (BTN1_W + 10))
			#define	BTN7_Y	BTN4_Y
			#define	BTN7_T	"步进频率-10"

/* 第3行按钮坐标 */
#define BTN8_H	BTN1_H
#define BTN8_W	BTN1_W
#define	BTN8_X	BTN1_X
#define	BTN8_Y	(BTN1_Y + 2 * (BTN1_H + 10))
#define	BTN8_T	"正转1步"

	#define BTN9_H	BTN1_H
	#define BTN9_W	BTN1_W
	#define	BTN9_X	(BTN1_X +  1 * (BTN1_W + 10))
	#define	BTN9_Y	BTN8_Y
	#define	BTN9_T	"反转1步"

		#define BTN10_H	BTN1_H
		#define BTN10_W	BTN1_W
		#define	BTN10_X	(BTN1_X +  2 * (BTN1_W + 10))
		#define	BTN10_Y	BTN8_Y
		#define	BTN10_T	"正转10步"

			#define BTN11_H	BTN1_H
			#define BTN11_W	BTN1_W
			#define	BTN11_X	(BTN1_X +  3 * (BTN1_W + 10))
			#define	BTN11_Y	BTN8_Y
			#define	BTN11_T	"反转10步"

/* 第4行按钮坐标 */
#define BTN12_H	BTN1_H
#define BTN12_W	BTN1_W
#define	BTN12_X	BTN1_X
#define	BTN12_Y	(BTN1_Y + 3 * (BTN1_H + 10))
#define	BTN12_T	"正转1/2圈"

	#define BTN13_H	BTN1_H
	#define BTN13_W	BTN1_W
	#define	BTN13_X	(BTN1_X +  1 * (BTN1_W + 10))
	#define	BTN13_Y	BTN12_Y
	#define	BTN13_T	"正转1圈"

		#define BTN14_H	BTN1_H
		#define BTN14_W	BTN1_W
		#define	BTN14_X	(BTN1_X +  2 * (BTN1_W + 10))
		#define	BTN14_Y	BTN12_Y
		#define	BTN14_T	"反转2圈"

			#define BTN15_H	BTN1_H
			#define BTN15_W	BTN1_W
			#define	BTN15_X	(BTN1_X +  3 * (BTN1_W + 10))
			#define	BTN15_Y	BTN12_Y
			#define	BTN15_T	"反转10圈"

#define	LBL1_X	BOX1_X + 5
#define	LBL1_Y	190
#define	LBL1_T "电机和SD卡不能同时用，需要插上J7跳线帽"

#define	LBL2_X	LBL1_X + 85
#define	LBL2_Y	LBL1_Y
#define	LBL2_T ""

#define	LBL3_X	LBL1_X
#define	LBL3_Y	LBL1_Y + 20
#define	LBL3_T "电机状态:"

#define	LBL4_X	LBL3_X + 85
#define	LBL4_Y	LBL3_Y
#define	LBL4_T " "


/* 按钮 */
/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H	32
#define BTN_RET_W	80
#define	BTN_RET_X	(g_LcdWidth - BTN_RET_W - 8)
#define	BTN_RET_Y	(g_LcdHeight - BTN_RET_H - 4)
#define	BTN_RET_T	"返回"

static void InitFormMoto(void);
static void DispFormMoto(void);
static void DispMotoInfo(void);

FormMoto_T *FormMoto;

/*
*********************************************************************************************************
*	函 数 名: FormMainMoto
*	功能说明: 步进电机测试主程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void FormMainMoto(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	FormMoto_T form;
	uint8_t fDispInfo = 1;
	uint32_t freq = 500;	/* 0.5KHz 起步， 1KHz启动时 无法启动 */

	FormMoto = &form;

	InitFormMoto();
	DispFormMoto();
	
	bsp_InitStepMoto();
	g_tMoto.StepFreq  = freq;
	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		if (fDispInfo)
		{
			fDispInfo = 0;

			DispMotoInfo();
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
					if (LCD_ButtonTouchDown(&FormMoto->BtnRet, tpX, tpY))
					{
						//fQuit = 1;	释放时再退出
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn1, tpX, tpY))
					{
						MOTO_Start(freq, 0, 0);	/* 正转 */
						fDispInfo = 1;
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn2, tpX, tpY))
					{
						MOTO_Start(freq, 1, 0);	/* 反转 */
						fDispInfo = 1;
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn3, tpX, tpY))
					{
						MOTO_Pause();		/* 暂停 */
						fDispInfo = 1;
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn4, tpX, tpY))
					{
						freq += 1;
						MOTO_ShangeSpeed(freq);
						fDispInfo = 1;
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn5, tpX, tpY))
					{
						if (freq > 0)
						{
							freq -= 1;
						}
						MOTO_ShangeSpeed(freq);
						fDispInfo = 1;
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn6, tpX, tpY))
					{
						freq += 10;
						MOTO_ShangeSpeed(freq);
						fDispInfo = 1;
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn7, tpX, tpY))	/* 步进频率 -100 */
					{
						if (freq > 10)
						{
							freq -= 10;
						}
						MOTO_ShangeSpeed(freq);
						fDispInfo = 1;
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn8, tpX, tpY))	/* 正转1步 */
					{
						MOTO_Start(freq, 0, 1);	/* 正转 */
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn9, tpX, tpY))	/* 反转1步 */
					{
						MOTO_Start(freq, 1, 1);	/* 反转 */
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn10, tpX, tpY))	/* 正转10步 */
					{
						MOTO_Start(freq, 0, 10);	/* 正转 */
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn11, tpX, tpY))	/* 反转10步 */
					{
						MOTO_Start(freq, 1, 10);	/* 反转 */
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn12, tpX, tpY))	/* 正转 1/2圈 */
					{
						MOTO_Start(freq, 0, MOTO_RoudToStep() / 2);
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn13, tpX, tpY))	/* 正转 1圈 */
					{
						MOTO_Start(freq, 0, MOTO_RoudToStep());
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn14, tpX, tpY))	/* 反转2 圈*/
					{
						MOTO_Start(freq, 1, MOTO_RoudToStep() * 2);
					}
					else if (LCD_ButtonTouchDown(&FormMoto->Btn15, tpX, tpY))	/* 反转 10 圈*/
					{
						MOTO_Start(freq, 1, MOTO_RoudToStep() * 10);
					}							
					else if (LCD_ButtonTouchDown(&FormMoto->Btn16, tpX, tpY))
					{
						MOTO_Stop();		/* 停止，线圈都不通电 */
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (LCD_ButtonTouchRelease(&FormMoto->BtnRet, tpX, tpY))
					{
						fQuit = 1;	/* 返回 */
					}
					else
					{
						LCD_ButtonTouchRelease(&FormMoto->BtnRet, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn1, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn2, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn3, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn4, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn5, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn6, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn7, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn8, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn9, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn10, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn11, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn12, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn13, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn14, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn15, tpX, tpY);
						LCD_ButtonTouchRelease(&FormMoto->Btn16, tpX, tpY);
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
				case KEY_DOWN_K1:		/* K1键 + 1*/
					break;

				case KEY_DOWN_K2:		/* K2键 - 1 */
					break;

				case KEY_DOWN_K3:		/* K3键 - 清0 */
					break;

				case JOY_DOWN_U:		/* 摇杆UP键按下 */
					break;

				case JOY_DOWN_D:		/* 摇杆DOWN键按下 */
					break;

				case JOY_DOWN_L:		/* 摇杆LEFT键按下 */
					break;

				case JOY_DOWN_R:		/* 摇杆RIGHT键按下 */
					break;

				case JOY_DOWN_OK:		/* 摇杆OK键按下 */
					break;

				default:
					break;
			}
		}
	}

	MOTO_Stop();	/* 停止电机驱动 */
}


/*
*********************************************************************************************************
*	函 数 名: InitFormMoto
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormMoto(void)
{
	/* 分组框标题字体 */
	FormMoto->FontBox.FontCode = FC_ST_16;
	FormMoto->FontBox.BackColor = CL_BTN_FACE;	/* 和背景色相同 */
	FormMoto->FontBox.FrontColor = CL_BLACK;
	FormMoto->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormMoto->FontBlack.FontCode = FC_ST_16;
	FormMoto->FontBlack.BackColor = CL_MASK;		/* 透明色 */
	FormMoto->FontBlack.FrontColor = CL_BLACK;
	FormMoto->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormMoto->FontBlue.FontCode = FC_ST_16;
	FormMoto->FontBlue.BackColor = CL_BTN_FACE;
	FormMoto->FontBlue.FrontColor = CL_BLUE;
	FormMoto->FontBlue.Space = 0;

	/* 按钮字体 */
	FormMoto->FontBtn.FontCode = FC_ST_16;
	FormMoto->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
	FormMoto->FontBtn.FrontColor = CL_BLACK;
	FormMoto->FontBtn.Space = 0;

	/* 分组框 */
	FormMoto->Box1.Left = BOX1_X;
	FormMoto->Box1.Top = BOX1_Y;
	FormMoto->Box1.Height = BOX1_H;
	FormMoto->Box1.Width = BOX1_W;
	FormMoto->Box1.pCaption = BOX1_T;
	FormMoto->Box1.Font = &FormMoto->FontBox;

	/* 标签 */
	FormMoto->Label1.Left = LBL1_X;
	FormMoto->Label1.Top = LBL1_Y;
	FormMoto->Label1.MaxLen = 0;
	FormMoto->Label1.pCaption = LBL1_T;
	FormMoto->Label1.Font = &FormMoto->FontBlack;

	FormMoto->Label2.Left = LBL2_X;
	FormMoto->Label2.Top = LBL2_Y;
	FormMoto->Label2.MaxLen = 0;
	FormMoto->Label2.pCaption = LBL2_T;
	FormMoto->Label2.Font = &FormMoto->FontBlue;

	FormMoto->Label3.Left = LBL3_X;
	FormMoto->Label3.Top = LBL3_Y;
	FormMoto->Label3.MaxLen = 0;
	FormMoto->Label3.pCaption = LBL3_T;
	FormMoto->Label3.Font = &FormMoto->FontBlack;

	FormMoto->Label4.Left = LBL4_X;
	FormMoto->Label4.Top = LBL4_Y;
	FormMoto->Label4.MaxLen = 0;
	FormMoto->Label4.pCaption = LBL4_T;
	FormMoto->Label4.Font = &FormMoto->FontBlue;

	/* 按钮 */
	FormMoto->BtnRet.Left = BTN_RET_X;
	FormMoto->BtnRet.Top = BTN_RET_Y;
	FormMoto->BtnRet.Height = BTN_RET_H;
	FormMoto->BtnRet.Width = BTN_RET_W;
	FormMoto->BtnRet.pCaption = BTN_RET_T;
	FormMoto->BtnRet.Font = &FormMoto->FontBtn;
	FormMoto->BtnRet.Focus = 0;

	FormMoto->Btn1.Left = BTN1_X;
	FormMoto->Btn1.Top = BTN1_Y;
	FormMoto->Btn1.Height = BTN1_H;
	FormMoto->Btn1.Width = BTN1_W;
	FormMoto->Btn1.pCaption = BTN1_T;
	FormMoto->Btn1.Font = &FormMoto->FontBtn;
	FormMoto->Btn1.Focus = 0;

	FormMoto->Btn2.Left = BTN2_X;
	FormMoto->Btn2.Top = BTN2_Y;
	FormMoto->Btn2.Height = BTN2_H;
	FormMoto->Btn2.Width = BTN2_W;
	FormMoto->Btn2.pCaption = BTN2_T;
	FormMoto->Btn2.Font = &FormMoto->FontBtn;
	FormMoto->Btn2.Focus = 0;

	FormMoto->Btn3.Left = BTN3_X;
	FormMoto->Btn3.Top = BTN3_Y;
	FormMoto->Btn3.Height = BTN3_H;
	FormMoto->Btn3.Width = BTN3_W;
	FormMoto->Btn3.pCaption = BTN3_T;
	FormMoto->Btn3.Font = &FormMoto->FontBtn;
	FormMoto->Btn3.Focus = 0;

	FormMoto->Btn4.Left = BTN4_X;
	FormMoto->Btn4.Top = BTN4_Y;
	FormMoto->Btn4.Height = BTN4_H;
	FormMoto->Btn4.Width = BTN4_W;
	FormMoto->Btn4.pCaption = BTN4_T;
	FormMoto->Btn4.Font = &FormMoto->FontBtn;
	FormMoto->Btn4.Focus = 0;

	FormMoto->Btn5.Left = BTN5_X;
	FormMoto->Btn5.Top = BTN5_Y;
	FormMoto->Btn5.Height = BTN5_H;
	FormMoto->Btn5.Width = BTN5_W;
	FormMoto->Btn5.pCaption = BTN5_T;
	FormMoto->Btn5.Font = &FormMoto->FontBtn;
	FormMoto->Btn5.Focus = 0;

	FormMoto->Btn6.Left = BTN6_X;
	FormMoto->Btn6.Top = BTN6_Y;
	FormMoto->Btn6.Height = BTN6_H;
	FormMoto->Btn6.Width = BTN6_W;
	FormMoto->Btn6.pCaption = BTN6_T;
	FormMoto->Btn6.Font = &FormMoto->FontBtn;
	FormMoto->Btn6.Focus = 0;

	FormMoto->Btn7.Left = BTN7_X;
	FormMoto->Btn7.Top = BTN7_Y;
	FormMoto->Btn7.Height = BTN7_H;
	FormMoto->Btn7.Width = BTN7_W;
	FormMoto->Btn7.pCaption = BTN7_T;
	FormMoto->Btn7.Font = &FormMoto->FontBtn;
	FormMoto->Btn7.Focus = 0;

	FormMoto->Btn8.Left = BTN8_X;
	FormMoto->Btn8.Top = BTN8_Y;
	FormMoto->Btn8.Height = BTN8_H;
	FormMoto->Btn8.Width = BTN8_W;
	FormMoto->Btn8.pCaption = BTN8_T;
	FormMoto->Btn8.Font = &FormMoto->FontBtn;
	FormMoto->Btn8.Focus = 0;

	FormMoto->Btn9.Left = BTN9_X;
	FormMoto->Btn9.Top = BTN9_Y;
	FormMoto->Btn9.Height = BTN9_H;
	FormMoto->Btn9.Width = BTN9_W;
	FormMoto->Btn9.pCaption = BTN9_T;
	FormMoto->Btn9.Font = &FormMoto->FontBtn;
	FormMoto->Btn9.Focus = 0;

	FormMoto->Btn10.Left = BTN10_X;
	FormMoto->Btn10.Top = BTN10_Y;
	FormMoto->Btn10.Height = BTN10_H;
	FormMoto->Btn10.Width = BTN10_W;
	FormMoto->Btn10.pCaption = BTN10_T;
	FormMoto->Btn10.Font = &FormMoto->FontBtn;
	FormMoto->Btn10.Focus = 0;

	FormMoto->Btn11.Left = BTN11_X;
	FormMoto->Btn11.Top = BTN11_Y;
	FormMoto->Btn11.Height = BTN11_H;
	FormMoto->Btn11.Width = BTN11_W;
	FormMoto->Btn11.pCaption = BTN11_T;
	FormMoto->Btn11.Font = &FormMoto->FontBtn;
	FormMoto->Btn11.Focus = 0;

	FormMoto->Btn12.Left = BTN12_X;
	FormMoto->Btn12.Top = BTN12_Y;
	FormMoto->Btn12.Height = BTN12_H;
	FormMoto->Btn12.Width = BTN12_W;
	FormMoto->Btn12.pCaption = BTN12_T;
	FormMoto->Btn12.Font = &FormMoto->FontBtn;
	FormMoto->Btn12.Focus = 0;

	FormMoto->Btn13.Left = BTN13_X;
	FormMoto->Btn13.Top = BTN13_Y;
	FormMoto->Btn13.Height = BTN13_H;
	FormMoto->Btn13.Width = BTN13_W;
	FormMoto->Btn13.pCaption = BTN13_T;
	FormMoto->Btn13.Font = &FormMoto->FontBtn;
	FormMoto->Btn13.Focus = 0;

	FormMoto->Btn14.Left = BTN14_X;
	FormMoto->Btn14.Top = BTN14_Y;
	FormMoto->Btn14.Height = BTN14_H;
	FormMoto->Btn14.Width = BTN14_W;
	FormMoto->Btn14.pCaption = BTN14_T;
	FormMoto->Btn14.Font = &FormMoto->FontBtn;
	FormMoto->Btn14.Focus = 0;

	FormMoto->Btn15.Left = BTN15_X;
	FormMoto->Btn15.Top = BTN15_Y;
	FormMoto->Btn15.Height = BTN15_H;
	FormMoto->Btn15.Width = BTN15_W;
	FormMoto->Btn15.pCaption = BTN15_T;
	FormMoto->Btn15.Font = &FormMoto->FontBtn;
	FormMoto->Btn15.Focus = 0;

	FormMoto->Btn16.Left = BTN16_X;
	FormMoto->Btn16.Top = BTN16_Y;
	FormMoto->Btn16.Height = BTN16_H;
	FormMoto->Btn16.Width = BTN16_W;
	FormMoto->Btn16.pCaption = BTN16_T;
	FormMoto->Btn16.Font = &FormMoto->FontBtn;
	FormMoto->Btn16.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispFormMoto
*	功能说明: 显示所有的静态控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFormMoto(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&FormMoto->Box1);

	LCD_DrawLabel(&FormMoto->Label1);
	LCD_DrawLabel(&FormMoto->Label2);
	LCD_DrawLabel(&FormMoto->Label3);
	LCD_DrawLabel(&FormMoto->Label4);

	/* 按钮 */
	LCD_DrawButton(&FormMoto->Btn1);
	LCD_DrawButton(&FormMoto->Btn2);
	LCD_DrawButton(&FormMoto->Btn3);
	LCD_DrawButton(&FormMoto->Btn4);
	LCD_DrawButton(&FormMoto->Btn5);
	LCD_DrawButton(&FormMoto->Btn6);
	LCD_DrawButton(&FormMoto->Btn5);
	LCD_DrawButton(&FormMoto->Btn6);
	LCD_DrawButton(&FormMoto->Btn7);
	LCD_DrawButton(&FormMoto->Btn8);
	LCD_DrawButton(&FormMoto->Btn9);
	LCD_DrawButton(&FormMoto->Btn10);
	LCD_DrawButton(&FormMoto->Btn11);
	LCD_DrawButton(&FormMoto->Btn12);
	LCD_DrawButton(&FormMoto->Btn13);
	LCD_DrawButton(&FormMoto->Btn14);
	LCD_DrawButton(&FormMoto->Btn15);
	LCD_DrawButton(&FormMoto->Btn16);	
	LCD_DrawButton(&FormMoto->BtnRet);
}

/*
*********************************************************************************************************
*	函 数 名: DispDacVoltage
*	功能说明: 显示电压
*	形    参: 无
*			  _
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMotoInfo(void)
{
	char buf[128];

	if (g_tMoto.Running == 1)
	{
		if (g_tMoto.Dir == 0)
		{
			strcpy(buf, "正转");
		}
		else
		{
			strcpy(buf, "反转");
		}
	}
	else
	{
		strcpy(buf, "停止");	
	}	
	sprintf(&buf[strlen(buf)], ", 步进频率=%dHz", g_tMoto.StepFreq);

	FormMoto->Label4.pCaption = buf;
	LCD_DrawLabel(&FormMoto->Label4);
}


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

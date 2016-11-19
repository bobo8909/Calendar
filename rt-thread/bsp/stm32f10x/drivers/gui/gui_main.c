/**********************************************************************************************************
*	                                  
*	模块名称 : 主界面  
*	文件名称 : ui_main.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期        作者           说明
*		
*		v1.0    2012-10-19  wangkai        初步实现
*
*	Copyright (C), 2010-2011, UP MCU 工作室
*
**********************************************************************************************************/
#include <stddef.h>
#include "GUI.h"
#include "ICONVIEW.h"
#include "TEXT.h"
#include "my_gui.h"

#define ID_TIMER_TIME       1
#define ID_TIMER_CHARGE     2
#define ID_TIMER_RECEPTION  3


//ICON_VIEM index选择
#define FUNC_SEL_MP3				0
#define FUNC_SEL_JPG				1
#define FUNC_SEL_TXT				2
#define FUNC_SEL_CLOCK			3
#define FUNC_SEL_T9INPUT		4
#define FUNC_SEL_LED_CTR		5
#define FUNC_SEL_WRITE			6
#define FUNC_SEL_SYSTEM			7
#define FUNC_SEL_INFRED			8
/*********************************************************************
*
*       _aBitmapItem
*/
static  GUI_CONST_STORAGE BITMAP_ITEM _aBitmapItem[] = {
  {&_bmmusic,    "MP3"    , "Use the diary"},
  {&_bmClock,   "Clock"   , "Adjust current time and date"},
  {&_bmDate,    "Date"    , "Use the diary"},
  //{&_bmEmail,   "Email"   , "Read an email"},
  {&_bmMarry,   "Game"   , "Play Game"},
  {&_bmSystem,  "System"  , "Change system settings"},
  {&_bmRead,    "Read"    , "Read a document"},
  {&_bmWrite,   "Write"   , "Write an email"},
 // {&_bmPassword,"Password", "Determine the system password"},
  {&_bmUSB,		"USB", "USB"},
  {&_bmRemote,  "Network" , "Select network"},
};


static  GUI_CONST_STORAGE GUI_BITMAP * _apbmCharge[] = {
  &_bmCharge1_27x14,
  &_bmCharge2_27x14,
  &_bmCharge3_27x14,
  &_bmCharge4_27x14,
  &_bmCharge5_27x14,
};

/************************************************************************************
*	函 数 名: static void _cbWin(WM_MESSAGE * pMsg) 
*	功能说明: 主界面回调函数
*   入口参数：窗口管理器消息
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
static void _cbWin(WM_MESSAGE * pMsg) {
  int NCode, Id, Sel;
  WM_HWIN hItem, hDlg;
        
  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      /* Id of widget */
    NCode = pMsg->Data.v;                 /* Notification code */
    switch (Id) {
    case GUI_ID_ICONVIEW0:
      switch (NCode) {
      case WM_NOTIFICATION_SEL_CHANGED:
        /*
        * Change widget text changing the selection 
        */
        Sel   = ICONVIEW_GetSel(pMsg->hWinSrc);
        hItem = WM_GetDialogItem(hDlg, GUI_ID_TEXT1);
        TEXT_SetText(hItem, _aBitmapItem[Sel].pExplanation);
        break;
      }
      break;
    }
    break;
		#if 0
  case MSG_MOVE:
    /*
    * Move toucan position and invalidate toucan area
    */
    yPosToucan += yAdd;
    if (yPosToucan >= 100) {
      yAdd = -yAdd;
      yPosToucan = 100;
    } else if (yPosToucan <= 20) {
      yAdd = -yAdd;
      yPosToucan = 20;
    }
    Rect.x0 = 146;
    Rect.y0 = yPosToucan - 5;
    Rect.x1 = Rect.x0 + _bmToucan.XSize - 1;
    Rect.y1 = Rect.y0 + _bmToucan.YSize - 1 + 5;
    WM_InvalidateRect(pMsg->hWin, &Rect);
    break;
		#endif
  case WM_PAINT:
    /*
    * Draw background and toucan
    */
    //GUI_DrawBitmap(&_bmHund, 0, 0);
	  GUI_DrawGradientV(0, 0, 240, 320, 0x994400, 0xaa7700);
    
    break;
  }
}

/************************************************************************************
*	函 数 名: static void _cbStatus(WM_MESSAGE * pMsg) 
*	功能说明: 主界面状态栏回调函数
*   入口参数：窗口管理器消息
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
static void _cbStatus(WM_MESSAGE * pMsg) {
  int xSize, ySize, Id, i, xPos;
  static int Time = 9 * 60;
  static int Charge = GUI_COUNTOF(_apbmCharge);
  static int Reception = 5;
  static WM_HTIMER hTimerTime;
  static WM_HTIMER hTimerCharge;
  static WM_HTIMER hTimerReception;
  WM_HWIN hWin;

  hWin = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_PRE_PAINT:
    GUI_MULTIBUF_Begin();
    break;
  case WM_POST_PAINT:
    GUI_MULTIBUF_End();
    break;
  case WM_CREATE:
    hTimerTime      = WM_CreateTimer(hWin, ID_TIMER_TIME,      100, 0);
    hTimerCharge    = WM_CreateTimer(hWin, ID_TIMER_CHARGE,    110, 0);
    hTimerReception = WM_CreateTimer(hWin, ID_TIMER_RECEPTION, 150, 0);
    break;
  case WM_DELETE:
    WM_DeleteTimer(hTimerTime);
    WM_DeleteTimer(hTimerCharge);
    WM_DeleteTimer(hTimerReception);
    break;
  case WM_TIMER:
    Id = WM_GetTimerId(pMsg->Data.v);
    switch (Id) {
    case ID_TIMER_TIME:
      Time += 1;
      if (Time == 24 * 60) {
        Time = 0;
      }
      break;
    case ID_TIMER_CHARGE:
      Charge -= 1;
      if (Charge < 0) {
        Charge = GUI_COUNTOF(_apbmCharge);
      }
      break;
    case ID_TIMER_RECEPTION:
      Reception -= 1;
      if (Reception == 0) {
        Reception = 5;
      }
      break;
    }
    WM_InvalidateWindow(hWin);
    WM_RestartTimer(pMsg->Data.v, 0);
    break;
  case WM_PAINT:
    xSize = WM_GetWindowSizeX(hWin);
    ySize = WM_GetWindowSizeY(hWin);
    //
    // Draw background
    //
    //GUI_SetColor(0x303030);
    //GUI_FillRect(0, 0, xSize - 1, ySize - 3);
	GUI_DrawGradientV(0, 0, xSize - 1, ySize - 3, 0x999999, 0x555555);
    GUI_SetColor(0x808080);
    GUI_DrawHLine(ySize - 2, 0, xSize - 1);
    GUI_SetColor(0x404040);
    GUI_DrawHLine(ySize - 1, 0, xSize - 1);
    //
    // Draw time
    //
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_13B_ASCII);
    GUI_GotoXY(200, 4);
    GUI_DispDecSpace(Time / 60, 2);
    GUI_DispChar(':');
    GUI_DispDec(Time % 60, 2);

	  GUI_GotoXY(6, 4);
	  GUI_DispString("UP MCU");
    //
    // Draw alarm icon
    //
    GUI_DrawBitmap(&_bmAlarm_16x16, 175, 3);
    //
    // Draw charge level
    //
    GUI_DrawBitmap(&_bmBatteryEmpty_27x14, 135, 4);
    if (Charge) {
      GUI_DrawBitmap(_apbmCharge[Charge - 1], 135, 4);
    }
    //
    // Draw level of reception
    //
    for (xPos = 100, i = 0; i < Reception; i++) {
      GUI_DrawVLine(xPos + i * 4 + 0, 13 - i * 2, 15);
      GUI_DrawVLine(xPos + i * 4 + 1, 13 - i * 2, 15);
    }
    GUI_SetColor(GUI_GRAY);
    for (; i < 5; i++) {
      GUI_DrawVLine(xPos + i * 4 + 0, 13 - i * 2, 15);
      GUI_DrawVLine(xPos + i * 4 + 1, 13 - i * 2, 15);
    }
 

    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

/************************************************************************************
*	函 数 名: void ui_MainTask(void* parameter)
*	功能说明: 用户界面主函数
*   入口参数：窗口管理器消息
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
extern  TouchCalibrate(void);
extern unsigned char touchCalibrated;/*1已经校准过，0未校准*/
void GUI_Main(void* parameter){

  volatile int i,  Add, Sel;
  WM_HWIN hWin, hText;
  
 
  GUI_Init();

  if (touchCalibrated==0)TouchCalibrate();
  
  #if (!defined(WIN32))
  if ((LCD_GetMirrorYEx(0) == 1) && (LCD_GetSwapXYEx(0) == 1) && (LCD_GetVXSizeEx(0) > LCD_GetXSizeEx(0))){
    GUI_SetOrg(320, 0); /* Temporarilly required, will be solved in future */
  }
  #endif
  WM_EnableMemdev(WM_HBKWIN);
	
	/*		创建状态栏		*/
  WM_CreateWindowAsChild(
    0,
    0,
    240,
    25,
    WM_HBKWIN, WM_CF_SHOW | WM_CF_MEMDEV, _cbStatus, 0
  );	
	
  WM_SetCallback(WM_HBKWIN, _cbWin);

  /*
  * Create iconview widget
  */
  hWin = ICONVIEW_CreateEx(12, 40, 229, 280, 
                           WM_HBKWIN, WM_CF_SHOW | WM_CF_HASTRANS, 
                           ICONVIEW_CF_AUTOSCROLLBAR_V, GUI_ID_ICONVIEW0, 55, 60);
  for (i = 0; i < GUI_COUNTOF(_aBitmapItem); i++) {
    /*
    * Add icons to the widget
    */
    ICONVIEW_AddBitmapItem(hWin, _aBitmapItem[i].pBitmap, _aBitmapItem[i].pText);
  }
  ICONVIEW_SetBkColor(hWin, ICONVIEW_CI_SEL, 0xdd7700 | 0x0C000000);//颜色高8位用于alpha混合
  ICONVIEW_SetFont(hWin, &GUI_Font13B_ASCII);
	
  ICONVIEW_SetSpace(hWin, GUI_COORD_X,18);//设置iconX方向距离
  ICONVIEW_SetSpace(hWin, GUI_COORD_Y,28);//设置iconY方向距离
	
  WM_SetFocus(hWin);
  Add = 1;
  Sel = 0;
	ICONVIEW_SetSel(hWin, Sel);
  
 
  while (1) {
    //GUI_Delay(2);/*会调用 GUI_EXEC*/ GUI_Delay 有点消耗cpu时间 咱等不起
	/* 原型如下
	void GUI_Delay(int Period) {
	int EndTime = GUI_GetTime()+Period;
	int tRem; 
	GUI_ASSERT_NO_LOCK();
	while (tRem = EndTime- GUI_GetTime(), tRem>0) {
		GUI_Exec();
		GUI_X_Delay((tRem >5) ? 5 : tRem);
	}
	}
	*/	
	GUI_Exec();  
	/*GUI_X_Delay(1);*/  
	GUI_X_ExecIdle();  
  }
}

/*************************** End of file ****************************/


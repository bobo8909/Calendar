/**********************************************************************************************************
*	                                  
*	模块名称 : 用户界面模块    
*	文件名称 : ui.h
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
#ifndef __UI_H
#define __UI_H

typedef struct {
  const GUI_BITMAP * pBitmap;
  const char       * pText;
  const char       * pExplanation;
} BITMAP_ITEM;

/*	声明程序用到的图标	*/
extern GUI_CONST_STORAGE GUI_BITMAP _bmClock;
extern GUI_CONST_STORAGE GUI_BITMAP _bmDate;
extern GUI_CONST_STORAGE GUI_BITMAP _bmMarry;//_bmEmail;
//extern GUI_CONST_STORAGE GUI_BITMAP _bmPassword;
extern GUI_CONST_STORAGE GUI_BITMAP _bmRead;
extern GUI_CONST_STORAGE GUI_BITMAP _bmRemote;
extern GUI_CONST_STORAGE GUI_BITMAP _bmSystem;
extern GUI_CONST_STORAGE GUI_BITMAP _bmWrite;
extern GUI_CONST_STORAGE GUI_BITMAP _bmmusic;
extern GUI_CONST_STORAGE GUI_BITMAP _bmUSB;

extern GUI_CONST_STORAGE GUI_BITMAP _bmAlarm_16x16;
extern GUI_CONST_STORAGE GUI_BITMAP * _apbmCharge[];
extern GUI_CONST_STORAGE GUI_COLOR ColorsBatteryEmpty_27x14[];
extern GUI_CONST_STORAGE GUI_BITMAP _bmBatteryEmpty_27x14;
extern GUI_CONST_STORAGE GUI_BITMAP _bmCharge1_27x14;
extern GUI_CONST_STORAGE GUI_BITMAP _bmCharge2_27x14;
extern GUI_CONST_STORAGE GUI_BITMAP _bmCharge3_27x14;
extern GUI_CONST_STORAGE GUI_BITMAP _bmCharge4_27x14;
extern GUI_CONST_STORAGE GUI_BITMAP _bmCharge5_27x14;
extern GUI_CONST_STORAGE GUI_BITMAP * _apbmCharge[];

void GUI_Main(void* parameter);

#endif//__UI_H

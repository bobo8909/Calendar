/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2012  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.16 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : SLIDER.h
Purpose     : SLIDER include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef SLIDER_H
#define SLIDER_H

#include "WM.h"
#include "DIALOG_Intern.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"      /* Req. for Create indirect data structure */

#if GUI_WINSUPPORT

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/************************************************************
*
*       Defines
*
*************************************************************
*/
/************************************************************
*
*       States
*/
#define SLIDER_STATE_PRESSED    WIDGET_STATE_USER0

/************************************************************
*
*       Create / Status flags
*/
#define SLIDER_CF_HORIZONTAL 0
#define SLIDER_CF_VERTICAL   WIDGET_CF_VERTICAL

/************************************************************
*
*       Skinning property indices
*/
#define SLIDER_SKINFLEX_PI_PRESSED   0
#define SLIDER_SKINFLEX_PI_UNPRESSED 1

/*********************************************************************
*
*       Public Types
*
**********************************************************************
*/
typedef WM_HMEM SLIDER_Handle;

typedef struct {
  GUI_COLOR aColorFrame[2];
  GUI_COLOR aColorInner[2];
  GUI_COLOR aColorShaft[3];
  GUI_COLOR ColorTick;
  GUI_COLOR ColorFocus;
  int TickSize;
  int ShaftSize;
} SLIDER_SKINFLEX_PROPS;

typedef struct {
  int Width;
  int NumTicks;
  int Size;
  int IsPressed;
  int IsVertical;
} SLIDER_SKINFLEX_INFO;

/*********************************************************************
*
*       Create functions
*
**********************************************************************
*/
SLIDER_Handle SLIDER_Create        (int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int WinFlags, int SpecialFlags);
SLIDER_Handle SLIDER_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);
SLIDER_Handle SLIDER_CreateEx      (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                    int WinFlags, int ExFlags, int Id);
SLIDER_Handle SLIDER_CreateUser    (int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                    int WinFlags, int ExFlags, int Id, int NumExtraBytes);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
void SLIDER_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Member functions
*
**********************************************************************
*/
void      SLIDER_Dec          (SLIDER_Handle hObj);
GUI_COLOR SLIDER_GetBarColor  (SLIDER_Handle hObj);
GUI_COLOR SLIDER_GetBkColor   (SLIDER_Handle hObj);
GUI_COLOR SLIDER_GetFocusColor(SLIDER_Handle hObj);
GUI_COLOR SLIDER_GetTickColor (SLIDER_Handle hObj);
int       SLIDER_GetUserData  (SLIDER_Handle hObj, void * pDest, int NumBytes);
void      SLIDER_Inc          (SLIDER_Handle hObj);
void      SLIDER_SetBarColor  (SLIDER_Handle hObj, GUI_COLOR Color);
void      SLIDER_SetBkColor   (SLIDER_Handle hObj, GUI_COLOR Color);
GUI_COLOR SLIDER_SetFocusColor(SLIDER_Handle hObj, GUI_COLOR Color);
void      SLIDER_SetNumTicks  (SLIDER_Handle hObj, int NumTicks);
void      SLIDER_SetRange     (SLIDER_Handle hObj, int Min, int Max);
void      SLIDER_SetTickColor (SLIDER_Handle hObj, GUI_COLOR Color);
int       SLIDER_SetUserData  (SLIDER_Handle hObj, const void * pSrc, int NumBytes);
void      SLIDER_SetValue     (SLIDER_Handle hObj, int v);
void      SLIDER_SetWidth     (SLIDER_Handle hObj, int Width);

/*********************************************************************
*
*       Member functions: Skinning
*
**********************************************************************
*/
void SLIDER_GetSkinFlexProps     (SLIDER_SKINFLEX_PROPS * pProps, int Index);
void SLIDER_SetSkinClassic       (SLIDER_Handle hObj);
void SLIDER_SetSkin              (SLIDER_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin);
int  SLIDER_DrawSkinFlex         (const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo);
void SLIDER_SetSkinFlexProps     (const SLIDER_SKINFLEX_PROPS * pProps, int Index);
void SLIDER_SetDefaultSkinClassic(void);
WIDGET_DRAW_ITEM_FUNC * SLIDER_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin);

#define SLIDER_SKIN_FLEX    SLIDER_DrawSkinFlex

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
GUI_COLOR SLIDER_GetDefaultBkColor   (void);
GUI_COLOR SLIDER_GetDefaultBarColor  (void);
GUI_COLOR SLIDER_GetDefaultFocusColor(void);
GUI_COLOR SLIDER_GetDefaultTickColor (void);
void      SLIDER_SetDefaultBkColor   (GUI_COLOR Color);
void      SLIDER_SetDefaultBarColor  (GUI_COLOR Color);
GUI_COLOR SLIDER_SetDefaultFocusColor(GUI_COLOR Color);
void      SLIDER_SetDefaultTickColor (GUI_COLOR Color);

/*********************************************************************
*
*       Query state
*
**********************************************************************
*/
int SLIDER_GetValue(SLIDER_Handle hObj);

#if defined(__cplusplus)
  }
#endif

#endif   /* if GUI_WINSUPPORT */
#endif   /* SLIDER_H */

/**********************************************************************************************************
*	                                  
*	模块名称 : GUI LCD驱动模块    
*	文件名称 : LCDConf.c
*	版    本 : V1.0
*	说    明 : 根据GUI格式要求实现LCD驱动 驱动芯片 ILI9325 
*	修改记录 :
*		版本号  日期        作者           说明
*		
*		v1.0    2012-10-11  UP MCU 工作室  初步实现
*
*	Copyright (C), 2010-2011, 
*
**********************************************************************************************************/
#include "GUI.h"
#include "GUIDRV_FlexColor.h"
#include "stm32f10x.h"
#include "LCDConf.h"
#include <rtthread.h>

//输出重定向.当不进行重定向时.
#define printf               rt_kprintf //使用rt_kprintf来输出
//#define printf(...) 

#define NEX_SELECT      FSMC_Bank1_NORSRAM2


void LCD_WR_REG(unsigned short index);
void LCD_WR_CMD(unsigned short index,unsigned short val);
void LCD_WR_Data(unsigned short val);
void LCD_WR_M_Data(U16 * pData, int NumWords);
void LCD_RD_M_Data(U16 * pData, int NumWords);
unsigned short LCD_RD_data(void);
void FSMC_LCD_Init(void);
void Delay(__IO uint32_t nCount);
/*********************************************************************
*
*       Layer configuration
*
**********************************************************************
*/
//
// 实际显示尺寸
//
#define XSIZE_PHYS 240
#define YSIZE_PHYS 320

//
// 颜色转换
//
#define COLOR_CONVERSION GUICC_565

//
// 显示驱动
//
#define DISPLAY_DRIVER GUIDRV_FLEXCOLOR  /*9320  9325*/

//
// Buffers / VScreens
//
#define NUM_BUFFERS   1
#define NUM_VSCREENS  1
//
// Display orientation
// 设置显示方向
//    #define DISPLAY_ORIENTATION  (GUI_MIRROR_X | GUI_MIRROR_Y) 
      #define DISPLAY_ORIENTATION  0//竖屏
 //     #define DISPLAY_ORIENTATION (GUI_MIRROR_X | GUI_MIRROR_Y)
//    #define DISPLAY_ORIENTATION (GUI_SWAP_XY | GUI_MIRROR_Y )//横屏
//    #define DISPLAY_ORIENTATION (GUI_SWAP_XY | GUI_MIRROR_X)
//
// Touch screen
//
#define USE_TOUCH   1

#define TOUCH_X_MIN 0xD5
#define TOUCH_X_MAX 0xF10  
#define TOUCH_Y_MIN 0xf27 
#define TOUCH_Y_MAX 0x120   


/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens and multiple buffers are not allowed!
#endif
#ifndef   DISPLAY_ORIENTATION
  #define DISPLAY_ORIENTATION  0
#endif

#if ((DISPLAY_ORIENTATION & GUI_SWAP_XY) != 0)
#define LANDSCAPE   1
#else
#define LANDSCAPE   0
#endif

#if (LANDSCAPE == 1)
#define WIDTH       YSIZE_PHYS  /* Screen Width (in pixels)         */
#define HEIGHT      XSIZE_PHYS  /* Screen Hight (in pixels)         */
#else
#define WIDTH       XSIZE_PHYS  /* Screen Width (in pixels)         */
#define HEIGHT      YSIZE_PHYS  /* Screen Hight (in pixels)         */
#endif

#if ((DISPLAY_ORIENTATION & GUI_SWAP_XY) != 0)
  #if ((DISPLAY_ORIENTATION & GUI_MIRROR_X) != 0)
    #define TOUCH_TOP    TOUCH_X_MAX
    #define TOUCH_BOTTOM TOUCH_X_MIN
  #else
    #define TOUCH_TOP    TOUCH_X_MIN
    #define TOUCH_BOTTOM TOUCH_X_MAX
  #endif
  #if ((DISPLAY_ORIENTATION & GUI_MIRROR_Y) != 0)
    #define TOUCH_LEFT   TOUCH_Y_MAX
    #define TOUCH_RIGHT  TOUCH_Y_MIN
  #else
    #define TOUCH_LEFT   TOUCH_Y_MIN
    #define TOUCH_RIGHT  TOUCH_Y_MAX
  #endif
#else
  #if ((DISPLAY_ORIENTATION & GUI_MIRROR_X) != 0)
    #define TOUCH_LEFT   TOUCH_X_MAX
    #define TOUCH_RIGHT  TOUCH_X_MIN
  #else
    #define TOUCH_LEFT   TOUCH_X_MIN
    #define TOUCH_RIGHT  TOUCH_X_MAX
  #endif
  #if ((DISPLAY_ORIENTATION & GUI_MIRROR_Y) != 0)
    #define TOUCH_TOP    TOUCH_Y_MAX
    #define TOUCH_BOTTOM TOUCH_Y_MIN
  #else
    #define TOUCH_TOP    TOUCH_Y_MIN
    #define TOUCH_BOTTOM TOUCH_Y_MAX
  #endif
#endif


/*以下函数为GUI使用 */
/************************************************************************************
*	函 数 名: FSMC_LCD_Init(void)
*	功能说明: LCD相关IO 和 FSMC接口配置
*   入口参数：无
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
void FSMC_LCD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  Timing_read,Timing_write;;

    /* Enable FSMC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);

    /* Set PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
       PD.10(D15), PD.14(D0), PD.15(D1) as alternate 
       function push pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
                                GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | 
                                GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Set PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
       PE.14(D11), PE.15(D12) as alternate function push pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* Set PF.00(A0 (RS)) as alternate function push pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* Set PG.9(NE2 (LCD/CS)) as alternate function push pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	/* Set PA.8调整背光 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    /*-- FSMC Configuration ------------------------------------------------------*/
    /*----------------------- SRAM Bank 4 ----------------------------------------*/
    /* FSMC_Bank1_NORSRAM4 configuration */
    /*-- FSMC Configuration -------------------------------------------------*/
	Timing_read.FSMC_AddressSetupTime = 0x01;	 //地址建立时间（ADDSET）为1个HCLK  
    Timing_read.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
    Timing_read.FSMC_DataSetupTime = 0x0f;		 //数据保存时间为16个HCLK,因为液晶驱动IC的读数据的时候，速度不能太快，尤其对SSD1289这个IC
    Timing_read.FSMC_BusTurnAroundDuration = 0x00;
    Timing_read.FSMC_CLKDivision = 0x00;
    Timing_read.FSMC_DataLatency = 0x00;
    Timing_read.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
	
 	Timing_write.FSMC_AddressSetupTime = 0x00;	 //地址建立时间（ADDSET）为1个HCLK  
    Timing_write.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
    Timing_write.FSMC_DataSetupTime = 0x03;		 ////数据保存时间为4个HCLK	
    Timing_write.FSMC_BusTurnAroundDuration = 0x00;
    Timing_write.FSMC_CLKDivision = 0x00;
    Timing_write.FSMC_DataLatency = 0x00;
    Timing_write.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
 
   /* Color LCD configuration ------------------------------------
     LCD configured as follow:
        - Data/Address MUX = Disable
        - Memory Type = SRAM
        - Data Width = 16bit
        - Write Operation = Enable
        - Extended Mode = Enable
        - Asynchronous Wait = Disable */
                                
  	FSMC_NORSRAMInitStructure.FSMC_Bank = NEX_SELECT;
  	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &Timing_read;
  	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &Timing_write;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  

    /* BANK 4 (of NOR/SRAM Bank 1~4) is enabled */
    FSMC_NORSRAMCmd(NEX_SELECT, ENABLE);
}

/*******************************************************************************
* Function Name  : LCD_ReadReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
*******************************************************************************/
static u16 LCD_ReadReg(u8 LCD_Reg)
{
   u16 i;
  /* Write 16-bit Index (then Read Reg) */
  *(__IO uint16_t *) (Bank1_LCD_C)= LCD_Reg;
  /* Read 16-bit Reg */
  i = *(__IO uint16_t *) (Bank1_LCD_D);	
  return (i);
}
/************************************************************************************
*	函 数 名: LCD_WR_REG(unsigned int index)
*	功能说明: FSMC写显示器寄存器地址函数
*   入口参数：显示寄存器地址
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
void LCD_WR_REG(unsigned short index)
{
	*(__IO uint16_t *) (Bank1_LCD_C)= index;

}
  
/************************************************************************************
*	函 数 名: LCD_WR_REG(unsigned int index)
*	功能说明: FSMC写显示器寄存器
*   入口参数：显示寄存器地址 需要写入的值
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
void LCD_WR_CMD(unsigned short index,unsigned short val)
{	
	*(__IO uint16_t *) (Bank1_LCD_C)= index;	
	*(__IO uint16_t *) (Bank1_LCD_D)= val;
}

/************************************************************************************
*	函 数 名: unsigned short LCD_RD_data(void)
*	功能说明: FSMC读显示区16位数据函数
*   入口参数：无
*   出口参数：16位颜色数据
*   说    明：
*   调用方法： 
*************************************************************************************/
unsigned short LCD_RD_data(void){
	unsigned int a=0;
	a=*(__IO uint16_t *) (Bank1_LCD_D);   //空操作
	a=*(__IO uint16_t *) (Bank1_LCD_D);   //读出的实际16位像素数据	  
	return(a);	
}

/************************************************************************************
*	函 数 名: LCD_WR_Data(unsigned int val)
*	功能说明: FSMC写16位数据函数
*   入口参数：需要写入的值
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
void LCD_WR_Data(unsigned short val)
{   
	*(__IO uint16_t *) (Bank1_LCD_D)= val; 	
}

/************************************************************************************
*	函 数 名: void LCD_WR_M_Data(U16 * pData, int NumWords)
*	功能说明: FSMC写多字节的16位数据函数
*   入口参数：写入的地址 ， 数据个数
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
void LCD_WR_M_Data(U16 * pData, int NumWords){
  for (; NumWords; NumWords--) {
    *(__IO uint16_t *) (Bank1_LCD_D)= *pData++;
  }
}


/************************************************************************************
*	函 数 名: void LCD_RD_M_Data(U16 * pData, int NumWords)
*	功能说明: FSMC读多字节的16位数据函数
*   入口参数：读出的地址 ， 数据个数
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
void LCD_RD_M_Data(U16 * pData, int NumWords){
  for (; NumWords; NumWords--){
    *pData++ = *(__IO uint16_t *) (Bank1_LCD_D); 
  }
}

/************************************************************************************
*	函 数 名: void Delay(__IO uint32_t nCount)
*	功能说明: 延时函数
*   入口参数：延时参数
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}

/************************************************************************************
*	函 数 名: static void _InitController(void) 
*	功能说明: 2.4寸屏(ILI9325  240X320)的初始化
*   入口参数：无
*   出口参数：无
*   说    明：先配置IO和FSMC 然后写各个控制寄存器
*   调用方法： 
*************************************************************************************/
vu16 lcdId;
static void _InitController(void) 
{

  FSMC_LCD_Init();

  LCD_WR_CMD(R0,0x0001);
  Delay(20);	
  lcdId = LCD_ReadReg(R0);

    /* deviceid check */
    if(
        (lcdId != 0x4531)
        && (lcdId != 0x7783)
        && (lcdId != 0x9320)
        && (lcdId != 0x9325)
        && (lcdId != 0x9328)
        && (lcdId != 0x9300)
		&& (lcdId != 0x1505)
    )
    {
        printf("Invalid LCD ID:%08X !!\r\n",lcdId);
        printf("Please check you hardware and configure.\r\n");
        //while(1);
		return ;
    }
    else
    {
        printf("\r\nLCD Device ID : %04X \r\n",lcdId);
    }
	
  if (lcdId==0x9320)
  {
		LCD_WR_CMD(0x00,0x0000);
		LCD_WR_CMD(0x01,0x0100);	//Driver Output Contral.
		LCD_WR_CMD(0x02,0x0700);	//LCD Driver Waveform Contral.
		//LCD_WR_CMD(0x03,0x0030);//Entry Mode Set.
		LCD_WR_CMD(0x03,0x1030);	//Entry Mode Set.
	    
	
		LCD_WR_CMD(0x04,0x0000);	//Scalling Contral.
		LCD_WR_CMD(0x08,0x0202);	//Display Contral 2.(0x0207)
		LCD_WR_CMD(0x09,0x0000);	//Display Contral 3.(0x0000)
		LCD_WR_CMD(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		LCD_WR_CMD(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		LCD_WR_CMD(0x0d,0x0000);	//Frame Maker Position.
		LCD_WR_CMD(0x0f,0x0000);	//Extern Display Interface Contral 2.	    
		Delay(50); 
		LCD_WR_CMD(0x07,0x0101);	//Display Contral.
		Delay(50); 								  
		LCD_WR_CMD(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
		LCD_WR_CMD(0x11,0x0007);								//Power Control 2.(0x0001)
		LCD_WR_CMD(0x12,(1<<8)|(1<<4)|(0<<0));				//Power Control 3.(0x0138)
		LCD_WR_CMD(0x13,0x0b00);								//Power Control 4.
		LCD_WR_CMD(0x29,0x0000);								//Power Control 7.
	
		LCD_WR_CMD(0x2b,(1<<14)|(1<<4));	    
		LCD_WR_CMD(0x50,0);	//Set X Star
		//水平GRAM终止位置Set X End.
		LCD_WR_CMD(0x51,239);	//Set Y Star
		LCD_WR_CMD(0x52,0);	//Set Y End.t.
		LCD_WR_CMD(0x53,319);	//
	
		LCD_WR_CMD(0x60,0x2700);	//Driver Output Control.
		LCD_WR_CMD(0x61,0x0001);	//Driver Output Control.
		LCD_WR_CMD(0x6a,0x0000);	//Vertical Srcoll Control.
	
		LCD_WR_CMD(0x80,0x0000);	//Display Position? Partial Display 1.
		LCD_WR_CMD(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		LCD_WR_CMD(0x82,0x0000);	//RAM Address End-Partial Display 1.
		LCD_WR_CMD(0x83,0x0000);	//Displsy Position? Partial Display 2.
		LCD_WR_CMD(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		LCD_WR_CMD(0x85,0x0000);	//RAM Address End? Partial Display 2.
	
		LCD_WR_CMD(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		LCD_WR_CMD(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		LCD_WR_CMD(0x93,0x0001);	//Panel Interface Contral 3.
		LCD_WR_CMD(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		LCD_WR_CMD(0x97,(0<<8));	//
		LCD_WR_CMD(0x98,0x0000);	//Frame Cycle Contral.	   
		LCD_WR_CMD(0x07,0x0173);	//(0x0173)

   }    
   else if( lcdId ==0x1505)
    {
		// second release on 3/5  ,luminance is acceptable,water wave appear during camera preview
        LCD_WR_CMD(0x0007,0x0000);
        Delay(50); 
        LCD_WR_CMD(0x0012,0x011C);//0x011A   why need to set several times?
        LCD_WR_CMD(0x00A4,0x0001);//NVM	 
        LCD_WR_CMD(0x0008,0x000F);
        LCD_WR_CMD(0x000A,0x0008);
        LCD_WR_CMD(0x000D,0x0008);	    
  		//伽马校正
        LCD_WR_CMD(0x0030,0x0707);
        LCD_WR_CMD(0x0031,0x0007); //0x0707
        LCD_WR_CMD(0x0032,0x0603); 
        LCD_WR_CMD(0x0033,0x0700); 
        LCD_WR_CMD(0x0034,0x0202); 
        LCD_WR_CMD(0x0035,0x0002); //?0x0606
        LCD_WR_CMD(0x0036,0x1F0F);
        LCD_WR_CMD(0x0037,0x0707); //0x0f0f  0x0105
        LCD_WR_CMD(0x0038,0x0000); 
        LCD_WR_CMD(0x0039,0x0000); 
        LCD_WR_CMD(0x003A,0x0707); 
        LCD_WR_CMD(0x003B,0x0000); //0x0303
        LCD_WR_CMD(0x003C,0x0007); //?0x0707
        LCD_WR_CMD(0x003D,0x0000); //0x1313//0x1f08
        Delay(50); 
        LCD_WR_CMD(0x0007,0x0001);
        LCD_WR_CMD(0x0017,0x0001);//开启电源
        Delay(50); 
  		//电源配置
        LCD_WR_CMD(0x0010,0x17A0); 
        LCD_WR_CMD(0x0011,0x0217);//reference voltage VC[2:0]   Vciout = 1.00*Vcivl
        LCD_WR_CMD(0x0012,0x011E);//0x011c  //Vreg1out = Vcilvl*1.80   is it the same as Vgama1out ?
        LCD_WR_CMD(0x0013,0x0F00);//VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl
        LCD_WR_CMD(0x002A,0x0000);  
        LCD_WR_CMD(0x0029,0x000A);//0x0001F  Vcomh = VCM1[4:0]*Vreg1out    gate source voltage??
        LCD_WR_CMD(0x0012,0x013E);// 0x013C  power supply on
        //Coordinates Control//
        LCD_WR_CMD(0x0050,0x0000);//0x0e00
        LCD_WR_CMD(0x0051,0x00EF); 
        LCD_WR_CMD(0x0052,0x0000); 
        LCD_WR_CMD(0x0053,0x013F); 
    	//Pannel Image Control//
        LCD_WR_CMD(0x0060,0x2700); 
        LCD_WR_CMD(0x0061,0x0001); 
        LCD_WR_CMD(0x006A,0x0000); 
        LCD_WR_CMD(0x0080,0x0000); 
    	//Partial Image Control//
        LCD_WR_CMD(0x0081,0x0000); 
        LCD_WR_CMD(0x0082,0x0000); 
        LCD_WR_CMD(0x0083,0x0000); 
        LCD_WR_CMD(0x0084,0x0000); 
        LCD_WR_CMD(0x0085,0x0000); 
  		//Panel Interface Control//
        LCD_WR_CMD(0x0090,0x0013);//0x0010 frenqucy
        LCD_WR_CMD(0x0092,0x0300); 
        LCD_WR_CMD(0x0093,0x0005); 
        LCD_WR_CMD(0x0095,0x0000); 
        LCD_WR_CMD(0x0097,0x0000); 
        LCD_WR_CMD(0x0098,0x0000); 
  
        LCD_WR_CMD(0x0001,0x0100); 
        LCD_WR_CMD(0x0002,0x0700); 
        LCD_WR_CMD(0x0003,0x1038);//扫描方向 上->下  左->右 
        LCD_WR_CMD(0x0004,0x0000); 
        LCD_WR_CMD(0x000C,0x0000); 
        LCD_WR_CMD(0x000F,0x0000); 
        LCD_WR_CMD(0x0020,0x0000); 
        LCD_WR_CMD(0x0021,0x0000); 
        LCD_WR_CMD(0x0007,0x0021); 
        Delay(20);
        LCD_WR_CMD(0x0007,0x0061); 
        Delay(20);
        LCD_WR_CMD(0x0007,0x0173); 
        Delay(20);
    }

}

/*本函数非ucgui驱动必须，只为实现屏幕截图功能而实现*/
/* 读取指定地址的GRAM */
unsigned short lcd_read_gram(unsigned int x,unsigned int y)
{
    unsigned short temp;
    //lcd_SetCursor(x,y);
	LCD_WR_CMD(32,x);    /* 0-239 */
    LCD_WR_CMD(33,y);    /* 0-319 */
	
    //rw_data_prepare();
	*(__IO uint16_t *) (Bank1_LCD_C)= 34;
    /* dummy read */
    temp = LCD_RD_data();
    return temp;
}

/*本函数非ucgui驱动必须，只为实现屏幕截图功能而实现*/
static unsigned short BGR2RGB(unsigned short c)
{
    u16  r, g, b, rgb;

    b = (c>>0)  & 0x1f;
    g = (c>>5)  & 0x3f;
    r = (c>>11) & 0x1f;

    rgb =  (b<<11) + (g<<5) + (r<<0);

    return( rgb );
}

/*本函数非ucgui驱动必须，只为实现屏幕截图功能而实现*/
/*读出的数据按照rgb565方式排列*/
rt_uint16_t lcd_get_piex(rt_uint16_t x,rt_uint16_t y)
{
	rt_uint16_t rgb;
    if ( (lcdId ==0x9325)|| (lcdId ==0x9328)|| (lcdId ==0x9320) || (lcdId ==0x1505))
    {
        rgb = BGR2RGB( lcd_read_gram(x,y) );
    }
    else if( lcdId ==0x4531 )
    {
        rgb = lcd_read_gram(x,y);
    }	
	return rgb;
}

/************************************************************************************
*	函 数 名: LCD_X_Config(void)
*	功能说明: 显示器的驱动配置
*   入口参数：无
*   出口参数：无
*   说    明：
*   调用方法： 
*************************************************************************************/
void LCD_X_Config(void) 
{
  GUI_DEVICE * pDevice;
  GUI_PORT_API PortAPI = {0}; 
  CONFIG_FLEXCOLOR Config = {0};
  
  #if (NUM_BUFFERS > 1)
    GUI_MULTIBUF_Config(NUM_BUFFERS);
  #endif
  /* 设置第一层的显示驱动和颜色转换 */
  pDevice = GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
  /* 公共显示驱动配置 */
   LCD_SetSizeEx(0, WIDTH, HEIGHT);               // 实际显示像素
  if (LCD_GetSwapXY()) 
  {
    LCD_SetSizeEx (0, WIDTH, HEIGHT);
    LCD_SetVSizeEx(0, WIDTH * NUM_VSCREENS, HEIGHT);
  } 
  else 
  {
    LCD_SetSizeEx (0, WIDTH, HEIGHT);
    LCD_SetVSizeEx(0, WIDTH, HEIGHT * NUM_VSCREENS);
  }							     

  /* 设置命令和数据模式 */ 	   
  PortAPI.pfWrite16_A0  = LCD_WR_REG;
  PortAPI.pfWrite16_A1  = LCD_WR_Data;
  PortAPI.pfWriteM16_A1 = LCD_WR_M_Data;
  PortAPI.pfReadM16_A1  = LCD_RD_M_Data;
  GUIDRV_FlexColor_SetFunc(pDevice, &PortAPI, GUIDRV_FLEXCOLOR_F66708, GUIDRV_FLEXCOLOR_M16C0B16);
  /* 显示方向和偏移	*/ 
  Config.Orientation   = DISPLAY_ORIENTATION;
  Config.RegEntryMode  = 0;
  GUIDRV_FlexColor_Config(pDevice, &Config);
  #if (USE_TOUCH == 1)
  
   
   //GUI_TOUCH_SetOrientation(DISPLAY_ORIENTATION);
    //在触摸调整界面下列设置已经被设置
    /* 校准触摸屏 */  
    GUI_TOUCH_Calibrate(GUI_COORD_X, 0, WIDTH  - 1, TOUCH_LEFT, TOUCH_RIGHT);
    GUI_TOUCH_Calibrate(GUI_COORD_Y, 0, HEIGHT   - 1,TOUCH_TOP,  TOUCH_BOTTOM); 
  #endif
}

/************************************************************************************
*	函 数 名: LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData)
*	功能说明: 显示器的驱动配置
*   入口参数：
*			LayerIndex - 配置层的序号
*   		Cmd        - 请参考下面的switch语句的细节
*   		pData      - LCD_X_DATA指针结构
*  出口参数：< -1 - 错误
*     		   -1 - 命令不处理
*      		    0 - 正确
*   说    明：
*   调用方法： 
*************************************************************************************/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;

  GUI_USE_PARA(LayerIndex);
  GUI_USE_PARA(pData);

  switch (Cmd){  
  	case LCD_X_INITCONTROLLER: 
  	{	   
       _InitController();      //LCD初始化
       return 0;
    }
    default:  r = -1;
  }
  return r;
}

/*************************** End of file ****************************/

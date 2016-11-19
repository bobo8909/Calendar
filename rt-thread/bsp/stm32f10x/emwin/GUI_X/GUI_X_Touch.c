/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              礐/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUI_TOUCH_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/
#include <rtthread.h>
#include "stm32f10x.h"
#include "touch.h"


#define TOUCH_DEBUG 0/* 调试开关 */

#if TOUCH_DEBUG 
  #define printf               rt_kprintf /*使用rt_kprintf来输出*/
#else
  #define printf(...)                     /*无输出*/
#endif	

extern u16 TP_Irq(void);

#define XSIZE_PHYS 240
#define YSIZE_PHYS 320

#define SAMPLE_TIMES 10

void GUI_TOUCH_X_ActivateX(void) {
}

void GUI_TOUCH_X_ActivateY(void) {
}


int  GUI_TOUCH_X_MeasureY(void) 
{
	unsigned char t=0,t1,count=0;
	unsigned short int dataBuffer[SAMPLE_TIMES];//数据组
	unsigned short temp=0,X=0;	
	unsigned int sum;
 	
	while(/*TP_Irq()==0&&*/count<SAMPLE_TIMES)//循环读数N次
	{	   	  
		dataBuffer[count]=TPReadX();
		xpt2046_write(0x80);
		count++; 
	}  
	if(count==SAMPLE_TIMES)//一定要读到10次数据,否则丢弃
	{  
	    do//将数据X升序排列
		{	
			t1=0;		  
			for(t=0;t<count-1;t++)
			{
				if(dataBuffer[t]>dataBuffer[t+1])//升序排列
				{
					temp=dataBuffer[t+1];
					dataBuffer[t+1]=dataBuffer[t];
					dataBuffer[t]=temp;
					t1=1; 
				}  
			}
		}while(t1); 	    
		sum = 0;
		for(t=1; t<(SAMPLE_TIMES-1);t++)
		{
			sum+=dataBuffer[t];
		}
		
		X=sum/(SAMPLE_TIMES-2);//(dataBuffer[3]+dataBuffer[4]+dataBuffer[5])/3;	 
		
//		if(X<=3730&&Y<=3730) //个人的屏根据初始参数修改.
//		{
//			if(X>=330)X-=330;
//			else X=0;
//			if(Y>=420)Y-=420;
//			else Y=0;  
//			drawbigpoint(240-X/14,320-Y/10);	 
//		}  
	printf("X position is : %d .\r\n",X);
	
	}
	
	return(X); 	 
}

int  GUI_TOUCH_X_MeasureX(void) {
  	unsigned char t=0,t1,count=0;
	unsigned short int dataBuffer[SAMPLE_TIMES];//数据组
	unsigned short temp=0,Y=0;	
	unsigned int sum;
	
    while(/*TP_Irq()==0&&*/count<SAMPLE_TIMES)//循环读数N次
	{	   	  
		dataBuffer[count]=TPReadY();
		xpt2046_write(0x80);
		count++;  
	}  
	if(count==SAMPLE_TIMES)//一定要读到10次数据,否则丢弃
	{  
	    do//将数据X升序排列
		{	
			t1=0;		  
			for(t=0;t<count-1;t++)
			{
				if(dataBuffer[t]>dataBuffer[t+1])//升序排列
				{
					temp=dataBuffer[t+1];
					dataBuffer[t+1]=dataBuffer[t];
					dataBuffer[t]=temp;
					t1=1; 
				}  
			}
		}while(t1); 	
		sum = 0;
		for(t=1; t<(SAMPLE_TIMES-1);t++)
		{
			sum+=dataBuffer[t];
		}		
		Y=sum/(SAMPLE_TIMES-2);//(dataBuffer[3]+dataBuffer[4]+dataBuffer[5])/3;	  	
		
//		if(X<=3730&&Y<=3730) //个人的屏根据初始参数修改.
//		{
//			if(X>=330)X-=330;
//			else X=0;
//			if(Y>=420)Y-=420;
//			else Y=0;  
//			drawbigpoint(240-X/14,320-Y/10);	 
//		} 
	printf("Y position is : %d .\r\n",Y);	
	
	}
	
	return(Y); 	
}



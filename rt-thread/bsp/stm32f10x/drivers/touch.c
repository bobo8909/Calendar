/**********************************************************************************************************
*	                                  
*	模块名称 : 触摸驱动模块
*	文件名称 : touch.c
*	版    本 : V1.0
*	说    明 : 触摸芯片xtp2046驱动
*	修改记录 :
*		版本号  日期        作者           说明
*		
*		v1.0    2012-11-19  wangkai        
*
*	Copyright (C), 2010-2011, UP MCU 工作室
*   淘宝店：   http://shop73275611.taobao.com
*   QQ交流群： 258043068
*
**********************************************************************************************************/

#include "stm32f10x.h"
#include "board.h"
#include <rtthread.h>
#include "GUI.h"
#include "touch.h"


/* 定义了触摸芯片的SPI片选控制 */
#define TP_CS()  GPIO_ResetBits(GPIOC,GPIO_Pin_1)	  
#define TP_DCS() GPIO_SetBits(GPIOC,GPIO_Pin_1)

/****************************************************************************
* 名    称：void TP_Config(void)
* 功    能：TFT 触摸屏控制初始化
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void TP_Config(void) 
{ 
  GPIO_InitTypeDef  GPIO_InitStructure; 
  SPI_InitTypeDef   SPI_InitStructure; 

  /* SPI1 时钟使能 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE); 
 
  /* SPI1 SCK(PA5)、MISO(PA6)、MOSI(PA7) 设置 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//口线速度50MHZ
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	        //复用模式
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* SPI1 触摸芯片的片选控制设置 PC1 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//口线速度50MHZ 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			//推挽输出模式
  GPIO_Init(GPIOC, &GPIO_InitStructure);
   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			    //输入上拉模式
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
	
  /*由于没用到spi flash 为了使spi flash芯片不干扰触摸读取，在初始化触摸时将flash cs脚（PE5）设为高*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOE, &GPIO_InitStructure); 
  GPIO_SetBits(GPIOE,GPIO_Pin_5);	
	
   /* SPI1总线 配置 */ 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   //全双工  
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						   //主模式
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					   //8位
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;						   //时钟极性 空闲状态时，SCK保持低电平
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;						   //时钟相位 数据采样从第一个时钟边沿开始
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							   //软件产生NSS
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;  //波特率控制 SYSCLK/64
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   //数据高位在前
  SPI_InitStructure.SPI_CRCPolynomial = 7;							   //CRC多项式寄存器初始值为7 
  SPI_Init(SPI1, &SPI_InitStructure);
  
  /* SPI1 使能 */  
  SPI_Cmd(SPI1,ENABLE);  
}

/****************************************************************************
* 名    称：u16 TP_Irq(void)
* 功    能：判断触摸屏有按下否
* 入口参数：无
* 出口参数：0：有按下
* 说    明：1：无按下
* 调用方法：无 
****************************************************************************/
u16 TP_Irq(void)
{
	u16 i;
	i = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0);
	
	return i;

}
/****************************************************************************
* 名    称：unsigned char SPI_WriteByte(unsigned char data) 
* 功    能：SPI1 写函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/  
unsigned char SPI_WriteByte(unsigned char data) 
{ 
 unsigned char Data = 0; 

  //等待发送缓冲区空
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET); 
  // 发送一个字节  
  SPI_I2S_SendData(SPI1,data); 

   //等待是否接收到一个字节 
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET); 
  // 获得该字节
  Data = SPI_I2S_ReceiveData(SPI1); 

  // 返回收到的字节 
  return Data; 
}  

/****************************************************************************
* 名    称：void SpiDelay(unsigned int DelayCnt) 
* 功    能：SPI1 写延时函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/  
void SpiDelay(unsigned int DelayCnt)
{
 unsigned int i;
 for(i=0;i<DelayCnt;i++);
}
/****************************************************************************
* 名    称：void xpt2046_write(unsigned char data)
* 功    能：SPI1 写延时函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/  
void xpt2046_write(unsigned char data)
{
   TP_CS();	                        //选择XPT2046 
   SpiDelay(10);					//延时
   SPI_WriteByte(data);				//设置X轴读取标志
   TP_DCS(); 	
}
/****************************************************************************
* 名    称：u16 TPReadX(void) 
* 功    能：触摸屏X轴数据读出
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/  
u16 TPReadX(void)
{ 
   u16 x=0;
   TP_CS();	                        //选择XPT2046 
   SpiDelay(10);					//延时
   SPI_WriteByte(0x90);				//设置X轴读取标志
   SpiDelay(10);					//延时
   x=SPI_WriteByte(0x00);			//连续读取16位的数据 
   x<<=8;
   x+=SPI_WriteByte(0x00);
   SpiDelay(10);					//禁止XPT2046
   TP_DCS(); 					    								  
   x = x>>3;						//移位换算成12位的有效数据0-4095
   return (x);
}
/****************************************************************************
* 名    称：u16 TPReadY(void)
* 功    能：触摸屏Y轴数据读出
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
u16 TPReadY(void)
{
   u16 y=0;
   TP_CS();	                        //选择XPT2046 
   SpiDelay(10);					//延时
   SPI_WriteByte(0xD0);				//设置Y轴读取标志
   SpiDelay(10);					//延时
   y=SPI_WriteByte(0x00);			//连续读取16位的数据 
   y<<=8;
   y+=SPI_WriteByte(0x00);
   SpiDelay(10);					//禁止XPT2046
   TP_DCS(); 					    								  
   y = y>>3;						//移位换算成12位的有效数据0-4095
   return (y);
}

/****************************************************************************
* 名    称： void Touch_ThreadEntry(void* parameter)
* 功    能：
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
void Touch_ThreadEntry(void* parameter)
{
	TP_Config();
	xpt2046_write(0x80);
	
	while(1)
	{	
		GUI_TOUCH_Exec(); 
		rt_thread_delay(1);//一般10ms读一次
   }
}


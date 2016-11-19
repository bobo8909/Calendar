/**********************************************************************************************************
*	                                  
*	ģ������ : ��������ģ��
*	�ļ����� : touch.c
*	��    �� : V1.0
*	˵    �� : ����оƬxtp2046����
*	�޸ļ�¼ :
*		�汾��  ����        ����           ˵��
*		
*		v1.0    2012-11-19  wangkai        
*
*	Copyright (C), 2010-2011, UP MCU ������
*   �Ա��꣺   http://shop73275611.taobao.com
*   QQ����Ⱥ�� 258043068
*
**********************************************************************************************************/

#include "stm32f10x.h"
#include "board.h"
#include <rtthread.h>
#include "GUI.h"
#include "touch.h"


/* �����˴���оƬ��SPIƬѡ���� */
#define TP_CS()  GPIO_ResetBits(GPIOC,GPIO_Pin_1)	  
#define TP_DCS() GPIO_SetBits(GPIOC,GPIO_Pin_1)

/****************************************************************************
* ��    �ƣ�void TP_Config(void)
* ��    �ܣ�TFT ���������Ƴ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void TP_Config(void) 
{ 
  GPIO_InitTypeDef  GPIO_InitStructure; 
  SPI_InitTypeDef   SPI_InitStructure; 

  /* SPI1 ʱ��ʹ�� */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE); 
 
  /* SPI1 SCK(PA5)��MISO(PA6)��MOSI(PA7) ���� */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//�����ٶ�50MHZ
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	        //����ģʽ
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* SPI1 ����оƬ��Ƭѡ�������� PC1 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//�����ٶ�50MHZ 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			//�������ģʽ
  GPIO_Init(GPIOC, &GPIO_InitStructure);
   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			    //��������ģʽ
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
	
  /*����û�õ�spi flash Ϊ��ʹspi flashоƬ�����Ŵ�����ȡ���ڳ�ʼ������ʱ��flash cs�ţ�PE5����Ϊ��*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOE, &GPIO_InitStructure); 
  GPIO_SetBits(GPIOE,GPIO_Pin_5);	
	
   /* SPI1���� ���� */ 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   //ȫ˫��  
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						   //��ģʽ
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					   //8λ
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;						   //ʱ�Ӽ��� ����״̬ʱ��SCK���ֵ͵�ƽ
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;						   //ʱ����λ ���ݲ����ӵ�һ��ʱ�ӱ��ؿ�ʼ
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							   //�������NSS
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;  //�����ʿ��� SYSCLK/64
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   //���ݸ�λ��ǰ
  SPI_InitStructure.SPI_CRCPolynomial = 7;							   //CRC����ʽ�Ĵ�����ʼֵΪ7 
  SPI_Init(SPI1, &SPI_InitStructure);
  
  /* SPI1 ʹ�� */  
  SPI_Cmd(SPI1,ENABLE);  
}

/****************************************************************************
* ��    �ƣ�u16 TP_Irq(void)
* ��    �ܣ��жϴ������а��·�
* ��ڲ�������
* ���ڲ�����0���а���
* ˵    ����1���ް���
* ���÷������� 
****************************************************************************/
u16 TP_Irq(void)
{
	u16 i;
	i = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0);
	
	return i;

}
/****************************************************************************
* ��    �ƣ�unsigned char SPI_WriteByte(unsigned char data) 
* ��    �ܣ�SPI1 д����
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/  
unsigned char SPI_WriteByte(unsigned char data) 
{ 
 unsigned char Data = 0; 

  //�ȴ����ͻ�������
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET); 
  // ����һ���ֽ�  
  SPI_I2S_SendData(SPI1,data); 

   //�ȴ��Ƿ���յ�һ���ֽ� 
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET); 
  // ��ø��ֽ�
  Data = SPI_I2S_ReceiveData(SPI1); 

  // �����յ����ֽ� 
  return Data; 
}  

/****************************************************************************
* ��    �ƣ�void SpiDelay(unsigned int DelayCnt) 
* ��    �ܣ�SPI1 д��ʱ����
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/  
void SpiDelay(unsigned int DelayCnt)
{
 unsigned int i;
 for(i=0;i<DelayCnt;i++);
}
/****************************************************************************
* ��    �ƣ�void xpt2046_write(unsigned char data)
* ��    �ܣ�SPI1 д��ʱ����
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/  
void xpt2046_write(unsigned char data)
{
   TP_CS();	                        //ѡ��XPT2046 
   SpiDelay(10);					//��ʱ
   SPI_WriteByte(data);				//����X���ȡ��־
   TP_DCS(); 	
}
/****************************************************************************
* ��    �ƣ�u16 TPReadX(void) 
* ��    �ܣ�������X�����ݶ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/  
u16 TPReadX(void)
{ 
   u16 x=0;
   TP_CS();	                        //ѡ��XPT2046 
   SpiDelay(10);					//��ʱ
   SPI_WriteByte(0x90);				//����X���ȡ��־
   SpiDelay(10);					//��ʱ
   x=SPI_WriteByte(0x00);			//������ȡ16λ������ 
   x<<=8;
   x+=SPI_WriteByte(0x00);
   SpiDelay(10);					//��ֹXPT2046
   TP_DCS(); 					    								  
   x = x>>3;						//��λ�����12λ����Ч����0-4095
   return (x);
}
/****************************************************************************
* ��    �ƣ�u16 TPReadY(void)
* ��    �ܣ�������Y�����ݶ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/
u16 TPReadY(void)
{
   u16 y=0;
   TP_CS();	                        //ѡ��XPT2046 
   SpiDelay(10);					//��ʱ
   SPI_WriteByte(0xD0);				//����Y���ȡ��־
   SpiDelay(10);					//��ʱ
   y=SPI_WriteByte(0x00);			//������ȡ16λ������ 
   y<<=8;
   y+=SPI_WriteByte(0x00);
   SpiDelay(10);					//��ֹXPT2046
   TP_DCS(); 					    								  
   y = y>>3;						//��λ�����12λ����Ч����0-4095
   return (y);
}

/****************************************************************************
* ��    �ƣ� void Touch_ThreadEntry(void* parameter)
* ��    �ܣ�
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/
void Touch_ThreadEntry(void* parameter)
{
	TP_Config();
	xpt2046_write(0x80);
	
	while(1)
	{	
		GUI_TOUCH_Exec(); 
		rt_thread_delay(1);//һ��10ms��һ��
   }
}


/**********************************************************************************************************
*	                                  
*	模块名称 : TFT驱动模块
*	文件名称 : ili_lcd_general.c
*	版    本 : V1.0
*	说    明 : 驱动多款TFT屏幕
*	修改记录 :
*		版本号  日期        作者                       说明
*		
*		v1.0    2012-01-19  jiezhi320(UP MCU工作室)    根据rtt的原版进行了改动
*                                                      支持：ili9320 ili9325 ili9328  / LG4531/R61505 	
*
*	Copyright (C), 
*   淘宝店：   http://shop73275611.taobao.com
*   QQ交流群： 258043068
*
**********************************************************************************************************/
#include "ili_lcd_general.h"
#include "stm32f10x.h"
#include "board.h"

#include "rtthread.h"
#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#endif


//输出重定向
#define printf               rt_kprintf //使用rt_kprintf来输出
//#define printf(...)  

/* LCD is connected to the FSMC_Bank1_NOR/SRAM4 and NE4 is used as ship select signal */
/* RS <==> A0 */
#define LCD_REG              (*((volatile unsigned short *) 0x64000000)) /* RS = 0 */
#define LCD_RAM              (*((volatile unsigned short *) 0x64000002)) /* RS = 1 */
#define NEX_SELECT            FSMC_Bank1_NORSRAM2


//内联函数定义,用以提高性能
#ifdef __CC_ARM                			 /* ARM Compiler 	*/
#define lcd_inline   				static __inline
#elif defined (__ICCARM__)        		/* for IAR Compiler */
#define lcd_inline 					inline
#elif defined (__GNUC__)        		/* GNU GCC Compiler */
#define lcd_inline 					static __inline
#else
#define lcd_inline                  static
#endif


#define rw_data_prepare()               write_cmd(34)

static rt_uint16_t deviceid;     //设置一个静态变量用来保存LCD的ID 
struct rt_device _lcd_device;	 //设备框架结构体

static void delay(rt_uint32_t cnt)
{
    volatile rt_uint32_t dl;
    while(cnt--)
    {
        for(dl=0; dl<500; dl++);
    }
}
/* 总线配置*/               
static void LCD_FSMCConfig(void)
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
	
	/* Set PA.8 打开背光 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
   
    /*-- FSMC Configuration -------------------------------------------------*/
	Timing_read.FSMC_AddressSetupTime = 0x03;	 //地址建立时间（ADDSET）为1个HCLK  
    Timing_read.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
    Timing_read.FSMC_DataSetupTime = 0x04;		 //数据保存时间为16个HCLK,因为液晶驱动IC的读数据的时候，速度不能太快
    Timing_read.FSMC_BusTurnAroundDuration = 0x00;
    Timing_read.FSMC_CLKDivision = 0x00;
    Timing_read.FSMC_DataLatency = 0x00;
    Timing_read.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
	
 	Timing_write.FSMC_AddressSetupTime = 0x03;	 //地址建立时间（ADDSET）为1个HCLK  
    Timing_write.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
    Timing_write.FSMC_DataSetupTime = 0x03;		 //数据保存时间为4个HCLK	
    Timing_write.FSMC_BusTurnAroundDuration = 0x00;
    Timing_write.FSMC_CLKDivision = 0x00;
    Timing_write.FSMC_DataLatency = 0x00;
    Timing_write.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
                              
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

static void lcd_port_init(void)
{
    LCD_FSMCConfig();
}

lcd_inline void write_cmd(rt_uint16_t cmd)
{
    LCD_REG = cmd;
}

lcd_inline rt_uint16_t read_data(void)
{
    return LCD_RAM;
}

lcd_inline void write_data(rt_uint16_t data_code )
{
    LCD_RAM = data_code;
}

lcd_inline void write_reg(rt_uint16_t reg_addr, rt_uint16_t reg_val)
{
    write_cmd(reg_addr);
    write_data(reg_val);
}

lcd_inline unsigned short read_reg(rt_uint16_t reg_addr)
{
    rt_uint16_t val=0;
    write_cmd(reg_addr);
    val = read_data();
    return (val);
}


//返回LCD的ID
rt_uint32_t lcd_getdeviceid(void)
{
    return deviceid;
}

static rt_uint16_t BGR2RGB(rt_uint16_t c)
{
    u16  r, g, b, rgb;

    b = (c>>0)  & 0x1f;
    g = (c>>5)  & 0x3f;
    r = (c>>11) & 0x1f;

    rgb =  (b<<11) + (g<<5) + (r<<0);
    return( rgb );
}
/* 设置光标位置 */
static void lcd_SetCursor(rt_uint32_t x, rt_uint32_t y)
{
#if defined(_ILI_HORIZONTAL_DIRECTION_)
	write_reg(32,y);    /* 0-239 */
    write_reg(33,x);    /* 0-319 */
#else
	write_reg(32,x);    /* 0-239 */
	write_reg(33,y);    /* 0-319 */
#endif	    
}

/* 读取指定地址的GRAM */
static unsigned short lcd_read_gram(rt_uint32_t x, rt_uint32_t y)
{
    rt_uint16_t temp;

    lcd_SetCursor(x,y);
    rw_data_prepare();
    /* dummy read */
    temp = read_data();
    temp = read_data();
    return temp;
}
#if 0
static void lcd_clear(rt_uint16_t Color)
{
    rt_uint32_t index=0;

    lcd_SetCursor(0,0);
    rw_data_prepare();         /* Prepare to write GRAM */
    for (index=0; index<(LCD_WIDTH*LCD_HEIGHT); index++)
    {
        write_data(Color);
    }
}
#endif 

static void lcd_data_bus_test(void)
{
    unsigned short temp1;
    unsigned short temp2;
    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
    write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (0<<3) );

    /* wirte */
    lcd_SetCursor(0,0);
    rw_data_prepare();
    write_data(0x5555);
    write_data(0xAAAA);

    /* read */
    lcd_SetCursor(0,0);
    if ( (deviceid ==0x9325)|| (deviceid ==0x9328)|| (deviceid ==0x9320) || (deviceid ==0x1505))
    {
        temp1 = BGR2RGB( lcd_read_gram(0,0) );
        temp2 = BGR2RGB( lcd_read_gram(1,0) );
    }
    else if( deviceid ==0x4531 )
    {
        temp1 = lcd_read_gram(0,0);
        temp2 = lcd_read_gram(1,0);
    }

    if( (temp1 == 0x5555) && (temp2 == 0xAAAA) )
    {
        printf(" data bus test pass!");
    }
    else
    {
        printf(" data bus test error: %04X %04X",temp1,temp2);
    }
}

static void lcd_gram_test(void)
{
    unsigned short temp;
    unsigned int test_x;
    unsigned int test_y;

    printf(" LCD GRAM test....");

    /* write */
    temp=0;
    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
    write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (0<<3) );
    lcd_SetCursor(0,0);
    rw_data_prepare();
    for(test_y=0; test_y<LCD_HEIGHT*LCD_WIDTH; test_y++)
    {
        write_data(temp);
        temp++;
    }
    /* read */
    temp=0;

    if ( (deviceid ==0x9320)|| (deviceid ==0x9325) || (deviceid ==0x9328) || (deviceid ==0x1505) )
    {
        for(test_y=0; test_y<LCD_HEIGHT; test_y++)
        {
            for(test_x=0; test_x<LCD_WIDTH; test_x++)
            {
                if( BGR2RGB( lcd_read_gram(test_x,test_y) ) != temp++)
                {
                    printf("  LCD GRAM ERR!!");
                    //while(1);
					return ;
                }
            }
        }
        printf("  TEST PASS!\r\n");
    }
    else if( deviceid ==0x4531 )
    {
        for(test_y=0; test_y<320; test_y++)
        {
            for(test_x=0; test_x<240; test_x++)
            {
                if(  lcd_read_gram(test_x,test_y) != temp++)
                {
                    printf("  LCD GRAM ERR!!");
                    //while(1);
					return ;
                }
            }
        }
        printf("  TEST PASS!\r\n");
    }
}


void lcd_Initializtion(void)
{
    lcd_port_init();
	
	delay(50); 
	write_reg(0x0000,0x0001);
	delay(50); 
    deviceid = read_reg(0x00);

    /* deviceid check */
    if(	(deviceid != 0x4531) && (deviceid != 0x7783) && (deviceid != 0x9320) && (deviceid != 0x9325)
        && (deviceid != 0x9328)	 && (deviceid != 0x9300)  	&& (deviceid != 0x1505))
    {
        printf("Invalid LCD ID:%08X\r\n!!",deviceid);
        printf("Please check you hardware and configure.");
        //while(1);
		return ;
    }
    else
    {
        printf("\r\nLCD Device ID : %04X ",deviceid);
    }

    if (deviceid==0x9325|| deviceid==0x9328)
    {
        write_reg(0x00e7,0x0010);
        write_reg(0x0000,0x0001);  			        //start internal osc
#if defined(_ILI_REVERSE_DIRECTION_)
        write_reg(0x0001,0x0000);                    //Reverse Display
#else
        write_reg(0x0001,0x0100);                    //
#endif
        write_reg(0x0002,0x0700); 				    //power on sequence
        /* [5:4]-ID1~ID0 [3]-AM-1垂直-0水平 */
        write_reg(0x0003,(1<<12)|(1<<5)|(0<<4) | (1<<3) );
        write_reg(0x0004,0x0000);
        write_reg(0x0008,0x0207);
        write_reg(0x0009,0x0000);
        write_reg(0x000a,0x0000); 				//display setting
        write_reg(0x000c,0x0001);				//display setting
        write_reg(0x000d,0x0000); 				//0f3c
        write_reg(0x000f,0x0000);
        //Power On sequence //
        write_reg(0x0010,0x0000);
        write_reg(0x0011,0x0007);
        write_reg(0x0012,0x0000);
        write_reg(0x0013,0x0000);
        delay(15);
        write_reg(0x0010,0x1590);
        write_reg(0x0011,0x0227);
        delay(15);
        write_reg(0x0012,0x009c);
        delay(15);
        write_reg(0x0013,0x1900);
        write_reg(0x0029,0x0023);
        write_reg(0x002b,0x000e);
        delay(15);
        write_reg(0x0020,0x0000);
        write_reg(0x0021,0x0000);
        delay(15);
        write_reg(0x0030,0x0007);
        write_reg(0x0031,0x0707);
        write_reg(0x0032,0x0006);
        write_reg(0x0035,0x0704);
        write_reg(0x0036,0x1f04);
        write_reg(0x0037,0x0004);
        write_reg(0x0038,0x0000);
        write_reg(0x0039,0x0706);
        write_reg(0x003c,0x0701);
        write_reg(0x003d,0x000f);
        delay(15);
        write_reg(0x0050,0x0000);
        write_reg(0x0051,0x00ef);
        write_reg(0x0052,0x0000);
        write_reg(0x0053,0x013f);
#if defined(_ILI_REVERSE_DIRECTION_)
        write_reg(0x0060,0x2700);
#else
        write_reg(0x0060,0xA700);
#endif
        write_reg(0x0061,0x0001);
        write_reg(0x006a,0x0000);
        write_reg(0x0080,0x0000);
        write_reg(0x0081,0x0000);
        write_reg(0x0082,0x0000);
        write_reg(0x0083,0x0000);
        write_reg(0x0084,0x0000);
        write_reg(0x0085,0x0000);
        write_reg(0x0090,0x0010);
        write_reg(0x0092,0x0000);
        write_reg(0x0093,0x0003);
        write_reg(0x0095,0x0110);
        write_reg(0x0097,0x0000);
        write_reg(0x0098,0x0000);
        //display on sequence
        write_reg(0x0007,0x0133);
        write_reg(0x0020,0x0000);
        write_reg(0x0021,0x0000);
    }
    else if( deviceid==0x9320|| deviceid==0x9300)
    {
        write_reg(0x00,0x0000);
#if defined(_ILI_REVERSE_DIRECTION_)
        write_reg(0x0001,0x0100);                    //Reverse Display
#else
        write_reg(0x0001,0x0000);                    // Driver Output Contral.
#endif
        write_reg(0x02,0x0700);	//LCD Driver Waveform Contral.
		write_reg(0x03,0x1030);	//Entry Mode Set.
//        write_reg(0x03,0x1018);	//Entry Mode Set.

        write_reg(0x04,0x0000);	//Scalling Contral.
        write_reg(0x08,0x0202);	//Display Contral 2.(0x0207)
        write_reg(0x09,0x0000);	//Display Contral 3.(0x0000)
        write_reg(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
        write_reg(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
        write_reg(0x0d,0x0000);	//Frame Maker Position.
        write_reg(0x0f,0x0000);	//Extern Display Interface Contral 2.

        delay(15);
        write_reg(0x07,0x0101);	//Display Contral.
        delay(15);

        write_reg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
        write_reg(0x11,0x0007);								//Power Control 2.(0x0001)
        write_reg(0x12,(1<<8)|(1<<4)|(0<<0));					//Power Control 3.(0x0138)
        write_reg(0x13,0x0b00);								//Power Control 4.
        write_reg(0x29,0x0000);								//Power Control 7.

        write_reg(0x2b,(1<<14)|(1<<4));

        write_reg(0x50,0);		//Set X Start.
        write_reg(0x51,239);	//Set X End.
        write_reg(0x52,0);		//Set Y Start.
        write_reg(0x53,319);	//Set Y End.

#if defined(_ILI_REVERSE_DIRECTION_)
        write_reg(0x0060,0x2700);  //Driver Output Control.
#else
        write_reg(0x0060,0xA700);
#endif
        write_reg(0x61,0x0001);	//Driver Output Control.
        write_reg(0x6a,0x0000);	//Vertical Srcoll Control.

        write_reg(0x80,0x0000);	//Display Position? Partial Display 1.
        write_reg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
        write_reg(0x82,0x0000);	//RAM Address End-Partial Display 1.
        write_reg(0x83,0x0000);	//Displsy Position? Partial Display 2.
        write_reg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
        write_reg(0x85,0x0000);	//RAM Address End? Partial Display 2.

        write_reg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
        write_reg(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
        write_reg(0x93,0x0001);	//Panel Interface Contral 3.
        write_reg(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
        write_reg(0x97,(0<<8));	//
        write_reg(0x98,0x0000);	//Frame Cycle Contral.


        write_reg(0x07,0x0173);	//(0x0173)
    }
    else if( deviceid==0x4531 )
    {
        // Setup display
        write_reg(0x00,0x0001);
        write_reg(0x10,0x0628);
        write_reg(0x12,0x0006);
        write_reg(0x13,0x0A32);
        write_reg(0x11,0x0040);
        write_reg(0x15,0x0050);
        write_reg(0x12,0x0016);
        delay(15);
        write_reg(0x10,0x5660);
        delay(15);
        write_reg(0x13,0x2A4E);
#if defined(_ILI_REVERSE_DIRECTION_)
        write_reg(0x01,0x0100);
#else
        write_reg(0x01,0x0000);
#endif
        write_reg(0x02,0x0300);

        write_reg(0x03,0x1030);
//	    write_reg(0x03,0x1038);

        write_reg(0x08,0x0202);
        write_reg(0x0A,0x0000);
        write_reg(0x30,0x0000);
        write_reg(0x31,0x0402);
        write_reg(0x32,0x0106);
        write_reg(0x33,0x0700);
        write_reg(0x34,0x0104);
        write_reg(0x35,0x0301);
        write_reg(0x36,0x0707);
        write_reg(0x37,0x0305);
        write_reg(0x38,0x0208);
        write_reg(0x39,0x0F0B);
        delay(15);
        write_reg(0x41,0x0002);

#if defined(_ILI_REVERSE_DIRECTION_)
        write_reg(0x0060,0x2700);
#else
        write_reg(0x0060,0xA700);
#endif

        write_reg(0x61,0x0001);
        write_reg(0x90,0x0119);
        write_reg(0x92,0x010A);
        write_reg(0x93,0x0004);
        write_reg(0xA0,0x0100);
//	    write_reg(0x07,0x0001);
        delay(15);
//	    write_reg(0x07,0x0021);
        delay(15);
//	    write_reg(0x07,0x0023);
        delay(15);
//	    write_reg(0x07,0x0033);
        delay(15);
        write_reg(0x07,0x0133);
        delay(15);
        write_reg(0xA0,0x0000);
        delay(20);
    }
    else if( deviceid ==0x7783)
    {
        // Start Initial Sequence
        write_reg(0x00FF,0x0001);
        write_reg(0x00F3,0x0008);
        write_reg(0x0001,0x0100);
        write_reg(0x0002,0x0700);
        write_reg(0x0003,0x1030);  //0x1030
        write_reg(0x0008,0x0302);
        write_reg(0x0008,0x0207);
        write_reg(0x0009,0x0000);
        write_reg(0x000A,0x0000);
        write_reg(0x0010,0x0000);  //0x0790
        write_reg(0x0011,0x0005);
        write_reg(0x0012,0x0000);
        write_reg(0x0013,0x0000);
        delay(20);
        write_reg(0x0010,0x12B0);
        delay(20);
        write_reg(0x0011,0x0007);
        delay(20);
        write_reg(0x0012,0x008B);
        delay(20);
        write_reg(0x0013,0x1700);
        delay(20);
        write_reg(0x0029,0x0022);

        //################# void Gamma_Set(void) ####################//
        write_reg(0x0030,0x0000);
        write_reg(0x0031,0x0707);
        write_reg(0x0032,0x0505);
        write_reg(0x0035,0x0107);
        write_reg(0x0036,0x0008);
        write_reg(0x0037,0x0000);
        write_reg(0x0038,0x0202);
        write_reg(0x0039,0x0106);
        write_reg(0x003C,0x0202);
        write_reg(0x003D,0x0408);
        delay(20);
        write_reg(0x0050,0x0000);
        write_reg(0x0051,0x00EF);
        write_reg(0x0052,0x0000);
        write_reg(0x0053,0x013F);
        write_reg(0x0060,0xA700);
        write_reg(0x0061,0x0001);
        write_reg(0x0090,0x0033);
        write_reg(0x002B,0x000B);
        write_reg(0x0007,0x0133);
        delay(20);
    }
    else if( deviceid ==0x1505)
    {
		// second release on 3/5  ,luminance is acceptable,water wave appear during camera preview
        write_reg(0x0007,0x0000);
        delay(50); 
        write_reg(0x0012,0x011C);//0x011A   why need to set several times?
        write_reg(0x00A4,0x0001);//NVM	 
        write_reg(0x0008,0x000F);
        write_reg(0x000A,0x0008);
        write_reg(0x000D,0x0008);	    
  		//伽马校正
        write_reg(0x0030,0x0707);
        write_reg(0x0031,0x0007); //0x0707
        write_reg(0x0032,0x0603); 
        write_reg(0x0033,0x0700); 
        write_reg(0x0034,0x0202); 
        write_reg(0x0035,0x0002); //?0x0606
        write_reg(0x0036,0x1F0F);
        write_reg(0x0037,0x0707); //0x0f0f  0x0105
        write_reg(0x0038,0x0000); 
        write_reg(0x0039,0x0000); 
        write_reg(0x003A,0x0707); 
        write_reg(0x003B,0x0000); //0x0303
        write_reg(0x003C,0x0007); //?0x0707
        write_reg(0x003D,0x0000); //0x1313//0x1f08
        delay(50); 
        write_reg(0x0007,0x0001);
        write_reg(0x0017,0x0001);//开启电源
        delay(50); 
  		//电源配置
        write_reg(0x0010,0x17A0); 
        write_reg(0x0011,0x0217);//reference voltage VC[2:0]   Vciout = 1.00*Vcivl
        write_reg(0x0012,0x011E);//0x011c  //Vreg1out = Vcilvl*1.80   is it the same as Vgama1out ?
        write_reg(0x0013,0x0F00);//VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl
        write_reg(0x002A,0x0000);  
        write_reg(0x0029,0x000A);//0x0001F  Vcomh = VCM1[4:0]*Vreg1out    gate source voltage??
        write_reg(0x0012,0x013E);// 0x013C  power supply on
        //Coordinates Control//
        write_reg(0x0050,0x0000);//0x0e00
        write_reg(0x0051,0x00EF); 
        write_reg(0x0052,0x0000); 
        write_reg(0x0053,0x013F); 
    	//Pannel Image Control//		
#if defined(_ILI_REVERSE_DIRECTION_)
        write_reg(0x0060,0x2700);
#else
        write_reg(0x0060,0xA700);
#endif		
	    write_reg(0x0061,0x0001); 	
		
        write_reg(0x006A,0x0000); 
        write_reg(0x0080,0x0000); 
    	//Partial Image Control//
        write_reg(0x0081,0x0000); 
        write_reg(0x0082,0x0000); 
        write_reg(0x0083,0x0000); 
        write_reg(0x0084,0x0000); 
        write_reg(0x0085,0x0000); 
  		//Panel Interface Control//
        write_reg(0x0090,0x0013);//0x0010 frenqucy
        write_reg(0x0092,0x0300); 
        write_reg(0x0093,0x0005); 
        write_reg(0x0095,0x0000); 
        write_reg(0x0097,0x0000); 
        write_reg(0x0098,0x0000); 
  
        write_reg(0x0001,0x0100); 
        write_reg(0x0002,0x0700); 
        write_reg(0x0003,0x1038);//扫描方向 上->下  左->右 
        write_reg(0x0004,0x0000); 
        write_reg(0x000C,0x0000); 
        write_reg(0x000F,0x0000); 
        write_reg(0x0020,0x0000); 
        write_reg(0x0021,0x0000); 
        write_reg(0x0007,0x0021); 
        delay(20);
        write_reg(0x0007,0x0061); 
        delay(20);
        write_reg(0x0007,0x0173); 
        delay(20);
    }
    //数据总线测试,用于测试硬件连接是否正常.
    lcd_data_bus_test();
    //GRAM测试,此测试可以测试LCD控制器内部GRAM.测试通过保证硬件正常
    lcd_gram_test();

    //清屏
    //lcd_clear( Blue );
}

void rt_hw_lcd_update(rtgui_rect_t *rect)
{
    /* nothing for none-DMA mode driver */
}

rt_uint8_t * rt_hw_lcd_get_framebuffer(void)
{
    return RT_NULL; /* no framebuffer driver */
}

/*  设置像素点 颜色,X,Y */
void rt_hw_lcd_set_pixel(rtgui_color_t *c, rt_base_t x, rt_base_t y)
{
    lcd_SetCursor(x,y);

    rw_data_prepare();
    write_data(*(rt_uint16_t*)c);
}

/* 获取像素点颜色 */
void rt_hw_lcd_get_pixel(rtgui_color_t *c, rt_base_t x, rt_base_t y)
{
	 if ( (deviceid ==0x9325) || (deviceid ==0x9328)|| (deviceid ==0x9320) || (deviceid ==0x1505))
	 {
      	*(rt_uint16_t*)c = BGR2RGB(lcd_read_gram(x,y));  
     }
     else	if( deviceid ==0x4531 )
	 {
	 	 *(rt_uint16_t*)c = lcd_read_gram(x,y);  
	 }
}


/* 画水平线 */
void rt_hw_lcd_draw_hline(const char* pixel, int x1, int x2, int y)
{
    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
	#if defined(_ILI_HORIZONTAL_DIRECTION_)
	write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (1<<3) );
	#else
	write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (0<<3) );
	#endif	

    lcd_SetCursor(x1, y);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (x1 < x2)
    {
        write_data(*(rt_uint16_t*)pixel);
        x1++;
    }
}

/* 垂直线 */
void rt_hw_lcd_draw_vline(const char* pixel, int x, int y1, int y2)
{
     /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
	#if defined(_ILI_HORIZONTAL_DIRECTION_)
	write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (0<<3) );
	#else
	write_reg(0x0003,(1<<12)|(1<<5)|(0<<4) | (1<<3) );
	#endif

    lcd_SetCursor(x, y1);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (y1 < y2)
    {
        write_data( *(rt_uint16_t*)pixel);
        y1++;
    }
}

/* ?? */
void rt_hw_lcd_draw_blit_line(const char* pixels, int x, int y, rt_size_t size)
{
    rt_uint16_t *ptr;
    ptr = (rt_uint16_t*)pixels;

    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
	#if defined(_ILI_HORIZONTAL_DIRECTION_)
	write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (1<<3) );
	#else
	write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (0<<3) );
	#endif

    lcd_SetCursor(x, y);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (size)
    {
        write_data(*ptr ++);
        size--;
    }
}

/* 下面的东东，就有linux 设备驱动框架的味道了 */
struct rt_device_graphic_ops lcd_ili_ops =
{
    rt_hw_lcd_set_pixel,
    rt_hw_lcd_get_pixel,
    rt_hw_lcd_draw_hline,
    rt_hw_lcd_draw_vline,
    rt_hw_lcd_draw_blit_line
};


static rt_err_t lcd_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t lcd_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t lcd_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t lcd_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    switch (cmd)
    {
    case RTGRAPHIC_CTRL_GET_INFO:
    {
        struct rt_device_graphic_info *info;

        info = (struct rt_device_graphic_info*) args;
        RT_ASSERT(info != RT_NULL);

        info->bits_per_pixel = 16;
        info->pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565P;
        info->framebuffer = RT_NULL;
	#if defined(_ILI_HORIZONTAL_DIRECTION_)
        info->width = LCD_HEIGHT;
        info->height = LCD_WIDTH;
	#else
		info->width = LCD_WIDTH;
        info->height = LCD_HEIGHT;
	#endif
    }
    break;

    case RTGRAPHIC_CTRL_RECT_UPDATE:
        /* nothong to be done */
        break;

    default:
        break;
    }

    return RT_EOK;
}

/* 需直接调用的 用于硬件初始化和注册设备 */
void rt_hw_lcd_init(void)
{
    /* register lcd device */
    _lcd_device.type  = RT_Device_Class_Graphic;
    _lcd_device.init  = lcd_init;
    _lcd_device.open  = lcd_open;
    _lcd_device.close = lcd_close;
    _lcd_device.control = lcd_control;
    _lcd_device.read  = RT_NULL;
    _lcd_device.write = RT_NULL;

    _lcd_device.user_data = &lcd_ili_ops;

    lcd_Initializtion();

    /* register graphic device driver */
    rt_device_register(&_lcd_device, "lcd",
                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}

/*以下提供finsh接口，用于调屏*/
#ifdef RT_USING_FINSH
#include "finsh.h"
void tft_write_reg(unsigned char reg_addr,unsigned short reg_val)
{
    write_cmd(reg_addr);
    write_data(reg_val);
}

FINSH_FUNCTION_EXPORT(tft_write_reg, addr & val ) ;
#endif RT_USING_FINSH

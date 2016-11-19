/**********************************************************************************************************
*	                                  
*	模块名称 : bmp截图模块
*	文件名称 : bmp.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期        作者                      说明
*		
*		v1.1    2013-03-19  jiezhi320(UP MCU工作室)    
*
*	Copyright (C), 2010-2013, 
*   淘宝店：   http://shop73275611.taobao.com
*   QQ交流群： 258043068
**********************************************************************************************************/
#include <dfs_posix.h>
#include <rtthread.h>
#include "stm32f10x.h"
#include "bmp.h"

/*
需要注意的是显示图像的顺序是 由下到上，由左到右。即像素数据
部分的第一像素数据是我们见到的图像的左下角的像素的数据；而像素数据的
最后一个有效数据是我们见到的图像的右上角的像素的数据。 
*/

//输出重定向
#define printf               rt_kprintf //使用rt_kprintf来输出
//#define printf(...)  

extern rt_uint16_t lcd_get_piex(rt_uint16_t x,rt_uint16_t y);


 /**********************************************************
 * 函数名：Screen_shot
 * 描述  ：截取LCD指定位置  指定宽高的像素 保存为24位真彩色bmp格式图片
 * 输入  : 	x								---水平位置 
 *					y								---竖直位置  
 *					Width						---水平宽度   
 *					Height					---竖直高度  	
 *					filename				---文件名
 * 输出  ：	0 		---成功
 *  				-1 		---失败
 *	    		8			---文件已存在
 * 举例  ：Screen_shot(0, 0, 320, 240, "/myScreen");-----全屏截图
 * 注意  ：x范围[0,319]  y范围[0,239]  Width范围[0,320-x]  Height范围[0,240-y]
 *					如果文件已存在,将直接返回	
 **************************************************************/    
rt_int32_t screen_shot(rt_uint16_t x, rt_uint16_t y, rt_uint16_t Width, rt_uint16_t Height,  rt_int8_t *filename)
{
	rt_int32_t fd_src;
	
    rt_uint8_t header[54] =
    {
        0x42, 0x4d, 0, 0, 0, 0, 
        0, 0, 0, 0, 54, 0, 
        0, 0, 40,0, 0, 0, 
        0, 0, 0, 0, 0, 0, 
        0, 0, 1, 0, 24, 0, 
        0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 
        0, 0, 0
    };
    volatile rt_uint32_t i;
    volatile rt_uint32_t j;
    volatile rt_uint32_t file_size;     
    volatile rt_uint32_t width;
    volatile rt_uint32_t height;
    volatile rt_uint16_t tmp_rgb;
    volatile rt_uint8_t rgb[3];	
    volatile rt_uint32_t mybw;
    volatile rt_int8_t kk[4]={0,0,0,0};
    
    file_size = (long)Width * (long)Height * 3 + Height*(Width%4) + sizeof(header);		//宽*高 +补充的字节 + 头部信息
    
    header[2] = (unsigned char)(file_size &0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;
    
    width=Width;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) &0x000000ff;
    header[20] = (width >> 16) &0x000000ff;
    header[21] = (width >> 24) &0x000000ff;
    
    height = Height;
    header[22] = height &0x000000ff;
    header[23] = (height >> 8) &0x000000ff;
    header[24] = (height >> 16) &0x000000ff;
    header[25] = (height >> 24) &0x000000ff;
    
    fd_src = open((const char*)filename, O_RDWR | O_TRUNC, 0);
    
    if ( fd_src >= 0)
    {    
       mybw = write(fd_src, header,sizeof(unsigned char)*54);
        for(i=0;i<Height;i++)			//高
        {
            if(!(Width%4))
            {
                for(j=0;j<Width;j++)  	//宽
                {   /*如果是横屏则 lcd_get_piex(Height-i+x,j+y);*/
                    tmp_rgb = lcd_get_piex(j+y,Height-i+x); 
                    rgb[2] =  GETR_FROM_RGB16(tmp_rgb);//r
                    rgb[1] =  GETG_FROM_RGB16(tmp_rgb);//g
                    rgb[0] =  GETB_FROM_RGB16(tmp_rgb);//b
					/*按照 b g r 顺序写入*/
                    mybw = write(fd_src, (const char *)rgb,sizeof(rgb));
                }
            }
            else
            {
                for(j=0;j<Width;j++)
                { 
					/*如果是横屏则 lcd_get_piex(Height-i+x,j+y);*/
                    tmp_rgb = lcd_get_piex(j+y,Height-i+x);
                    rgb[2] =  GETR_FROM_RGB16(tmp_rgb);//r
                    rgb[1] =  GETG_FROM_RGB16(tmp_rgb);//g
                    rgb[0] =  GETB_FROM_RGB16(tmp_rgb);//b  
					/*按照 b g r 顺序写入*/
                    mybw = write(fd_src, (const char *)rgb,sizeof(rgb));
                }
                mybw = write(fd_src, (const char *)kk,sizeof(unsigned char)*(Width%4));
            }	
        }
        close(fd_src); 
		printf("screen shot ok\r\n");
        return 0;
    }
    else 
	{	
		printf("save file list fail \r\n");
		return -1;
	}	
}



/*以下提供finsh接口，用于截屏*/
#ifdef RT_USING_FINSH
#include "finsh.h"

FINSH_FUNCTION_EXPORT(screen_shot, x y width heght filename ) ;
#endif RT_USING_FINSH

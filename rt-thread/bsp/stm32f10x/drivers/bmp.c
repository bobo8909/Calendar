/**********************************************************************************************************
*	                                  
*	ģ������ : bmp��ͼģ��
*	�ļ����� : bmp.c
*	��    �� : V1.0
*	˵    �� : 
*	�޸ļ�¼ :
*		�汾��  ����        ����                      ˵��
*		
*		v1.1    2013-03-19  jiezhi320(UP MCU������)    
*
*	Copyright (C), 2010-2013, 
*   �Ա��꣺   http://shop73275611.taobao.com
*   QQ����Ⱥ�� 258043068
**********************************************************************************************************/
#include <dfs_posix.h>
#include <rtthread.h>
#include "stm32f10x.h"
#include "bmp.h"

/*
��Ҫע�������ʾͼ���˳���� ���µ��ϣ������ҡ�����������
���ֵĵ�һ�������������Ǽ�����ͼ������½ǵ����ص����ݣ����������ݵ�
���һ����Ч���������Ǽ�����ͼ������Ͻǵ����ص����ݡ� 
*/

//����ض���
#define printf               rt_kprintf //ʹ��rt_kprintf�����
//#define printf(...)  

extern rt_uint16_t lcd_get_piex(rt_uint16_t x,rt_uint16_t y);


 /**********************************************************
 * ��������Screen_shot
 * ����  ����ȡLCDָ��λ��  ָ����ߵ����� ����Ϊ24λ���ɫbmp��ʽͼƬ
 * ����  : 	x								---ˮƽλ�� 
 *					y								---��ֱλ��  
 *					Width						---ˮƽ���   
 *					Height					---��ֱ�߶�  	
 *					filename				---�ļ���
 * ���  ��	0 		---�ɹ�
 *  				-1 		---ʧ��
 *	    		8			---�ļ��Ѵ���
 * ����  ��Screen_shot(0, 0, 320, 240, "/myScreen");-----ȫ����ͼ
 * ע��  ��x��Χ[0,319]  y��Χ[0,239]  Width��Χ[0,320-x]  Height��Χ[0,240-y]
 *					����ļ��Ѵ���,��ֱ�ӷ���	
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
    
    file_size = (long)Width * (long)Height * 3 + Height*(Width%4) + sizeof(header);		//��*�� +������ֽ� + ͷ����Ϣ
    
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
        for(i=0;i<Height;i++)			//��
        {
            if(!(Width%4))
            {
                for(j=0;j<Width;j++)  	//��
                {   /*����Ǻ����� lcd_get_piex(Height-i+x,j+y);*/
                    tmp_rgb = lcd_get_piex(j+y,Height-i+x); 
                    rgb[2] =  GETR_FROM_RGB16(tmp_rgb);//r
                    rgb[1] =  GETG_FROM_RGB16(tmp_rgb);//g
                    rgb[0] =  GETB_FROM_RGB16(tmp_rgb);//b
					/*���� b g r ˳��д��*/
                    mybw = write(fd_src, (const char *)rgb,sizeof(rgb));
                }
            }
            else
            {
                for(j=0;j<Width;j++)
                { 
					/*����Ǻ����� lcd_get_piex(Height-i+x,j+y);*/
                    tmp_rgb = lcd_get_piex(j+y,Height-i+x);
                    rgb[2] =  GETR_FROM_RGB16(tmp_rgb);//r
                    rgb[1] =  GETG_FROM_RGB16(tmp_rgb);//g
                    rgb[0] =  GETB_FROM_RGB16(tmp_rgb);//b  
					/*���� b g r ˳��д��*/
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



/*�����ṩfinsh�ӿڣ����ڽ���*/
#ifdef RT_USING_FINSH
#include "finsh.h"

FINSH_FUNCTION_EXPORT(screen_shot, x y width heght filename ) ;
#endif RT_USING_FINSH

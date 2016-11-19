/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
//#include <rtgui/calibration.h>
#endif
#include "uni2gb.h"
#include "GUI.h"
#include "led.h"
#include "touch.h"
#include "my_gui.h"
#include "setup.h"
#include "led.h"
extern void application_init(void);

/*bobby 20161112*/
#define LED1 0
#define LED2 1
#define LED3 2
#define LED4 3
const rt_uint8_t ledTable[4][4]=
{
	{0x00, 0x01, 0x02, 0x03},
	{0x01, 0x00, 0x02, 0x03},
	{0x02, 0x01, 0x00, 0x03},
	{0x03, 0x01, 0x02, 0x00},
};

/*bobby end*/
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t touch_stack[ 512 ];
static struct rt_thread touch_thread;

static rt_uint8_t led_stack[ 512 ];
static struct rt_thread led_thread;
static void led_thread_entry(void* parameter)
{
    unsigned int count=0;

    rt_hw_led_init();

    while (1)
    {
    
//	if (++count>=4) count = 0;
//	
//	rt_hw_led_on(ledTable[count][0]);
//	rt_hw_led_off(ledTable[count][1]);
//	rt_hw_led_off(ledTable[count][2]);
//	rt_hw_led_off(ledTable[count][3]);		
//	rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */
#if 1
		/* led1 on */
#ifndef RT_USING_FINSH
//        rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;
        rt_hw_led_on(LED1);
        rt_thread_delay( RT_TICK_PER_SECOND ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
//        rt_kprintf("led off\r\n");
#endif
        rt_hw_led_off(LED1);
        rt_thread_delay( RT_TICK_PER_SECOND );

		/*bobby 20161112*/
		/*led2 on*/		
        rt_hw_led_on(LED2);
        rt_thread_delay( RT_TICK_PER_SECOND ); /* sleep 0.5 second and switch to other thread */

		/*led2 off*/		
        rt_hw_led_off(LED2);
        rt_thread_delay( RT_TICK_PER_SECOND );

		/*led3 on*/		
        rt_hw_led_on(LED3);
        rt_thread_delay( RT_TICK_PER_SECOND ); /* sleep 0.5 second and switch to other thread */

		/*led3 off*/		
        rt_hw_led_off(LED3);
        rt_thread_delay( RT_TICK_PER_SECOND );

		/*led4 on*/		
        rt_hw_led_on(LED4);
        rt_thread_delay( RT_TICK_PER_SECOND ); /* sleep 0.5 second and switch to other thread */

		/*led4 off*/			
        rt_hw_led_off(LED4);
        rt_thread_delay( RT_TICK_PER_SECOND );
		/*bobby end*/
#endif
    }
}

#ifdef RT_USING_RTGUI
rt_bool_t cali_setup(void)
{
    rt_kprintf("cali setup entered\n");
    return RT_FALSE;
}

//void cali_store(struct calibration_data *data)
//{
//    rt_kprintf("cali finished (%d, %d), (%d, %d)\n",
//               data->min_x,
//               data->max_x,
//               data->min_y,
//               data->max_y);
//}
#endif /* RT_USING_RTGUI */

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
        rt_kprintf("File System initialzation failed!\n");
#endif  /* RT_USING_DFS */

#ifdef RT_USING_RTGUI
    {
        extern void rt_hw_lcd_init();
        extern void rtgui_touch_hw_init(void);

        rt_device_t lcd;

        /* init lcd */
        rt_hw_lcd_init();
		rt_kprintf("lcd_init\n");
		
        /* init touch panel */
        rtgui_touch_hw_init();
		rt_kprintf("touch_init\n");

        /* find lcd device */
        lcd = rt_device_find("lcd");

        /* set lcd device as rtgui graphic driver */
        rtgui_graphic_set_device(lcd);
		rt_kprintf("graphic\n");
		
#ifndef RT_USING_COMPONENTS_INIT
        /* init rtgui system server */
        rtgui_system_server_init();
#endif

//        calibration_set_restore(cali_setup);
//        calibration_set_after(cali_store);
//        calibration_init();
		application_init();	
    }
#endif /* #ifdef RT_USING_RTGUI */
}

int rt_application_init(void)
{
    rt_thread_t init_thread;
 	rt_thread_t gui_main_thread;
    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            2);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

  	result = rt_thread_init(&touch_thread,
		"touch",
		Touch_ThreadEntry, RT_NULL,
		(rt_uint8_t*)&touch_stack[0], sizeof(touch_stack), 10, 2);
	if (result == RT_EOK)
	{
        rt_thread_startup(&touch_thread);
	}	
	
	gui_main_thread = rt_thread_create("GUI_Main",
								GUI_Main, RT_NULL,
								4096, 15, 10);

	if (gui_main_thread != RT_NULL)
		rt_thread_startup(gui_main_thread);	
	

    return 0;
}

/*@}*/

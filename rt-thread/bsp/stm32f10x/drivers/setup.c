#include <rtthread.h>
#include <dfs_posix.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uni2gb.h"
#include "setup.h"     


#define setup_fn    "/resource/setup.ini"//配置文件存储路径

setup_TypeDef systemSetup;

static const char  kn_volume[]      = "default_volume";
static const char  kn_brightness[]  = "lcd_brightness";
static const char  kn_touch_min_x[] = "touch_min_x";
static const char  kn_touch_max_x[] = "touch_max_x";
static const char  kn_touch_min_y[] = "touch_min_y";
static const char  kn_touch_max_y[] = "touch_max_y";
 
static void load_default(void)
{
    rt_kprintf("load_default!\r\n");
    systemSetup.default_volume = 50;
    systemSetup.lcd_brightness = 90;

    systemSetup.touch_min_x = 0xD5;
    systemSetup.touch_max_x = 0xF10;
    systemSetup.touch_min_y = 0xf27;
    systemSetup.touch_max_y = 0x120;

    save_setup();
}

rt_err_t load_setup(void)
{
	extern unsigned char touchCalibrated;/*1已经校准过，0未校准*/
    int fd, length;
    char line[64];

    fd = open(setup_fn, O_RDONLY, 0);
    if (fd >= 0)
    {
        length = read_line(fd, line, sizeof(line));
        if (strcmp(line, "[config]") == 0)
        {
            char* begin;

            // default_volume
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_volume, sizeof(kn_volume) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                systemSetup.default_volume = atoi(begin);
            }

            // lcd_brightness
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_brightness, sizeof(kn_brightness) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                systemSetup.lcd_brightness = atoi(begin);
            }

            // touch_min_x
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
				touchCalibrated = 0;
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_min_x, sizeof(kn_touch_min_x) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                systemSetup.touch_min_x = atoi(begin);
				touchCalibrated = 1;
            }

            // touch_max_x
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_max_x, sizeof(kn_touch_max_x) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                systemSetup.touch_max_x = atoi(begin);
            }

            // touch_min_y
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_min_y, sizeof(kn_touch_min_y) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                systemSetup.touch_min_y = atoi(begin);
            }

            // touch_max_y
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_max_y, sizeof(kn_touch_max_y) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                systemSetup.touch_max_y = atoi(begin);
            }

        }
        else
        {
            close(fd);
            load_default();
            return RT_EOK;
        }
    }
    else
    {
        load_default();
    }

    close(fd);
    return RT_EOK;
}

rt_err_t save_setup(void)
{
    int fd, size;
    char* p_str;
    char* buf = rt_malloc(1024);

    if (buf == RT_NULL)
    {
        rt_kprintf("no memory\r\n");
        return RT_ENOMEM;
    }

    p_str = buf;

    //参数有效性检查,防止全黑或无声.
    if (systemSetup.default_volume < 5)systemSetup.default_volume = 5;
    if (systemSetup.lcd_brightness < 5)systemSetup.lcd_brightness = 5;

    fd = open(setup_fn, O_WRONLY | O_TRUNC, 0);
    if (fd >= 0)
    {
        size = sprintf(p_str, "[config]\r\n"); // [config] sprintf(p_str,"")
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_volume, systemSetup.default_volume); //default_volume
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_brightness, systemSetup.lcd_brightness); //lcd_brightness
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_min_x, systemSetup.touch_min_x); //touch_min_x
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_max_x, systemSetup.touch_max_x); //touch_max_x
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_min_y, systemSetup.touch_min_y); //touch_min_y
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_max_y, systemSetup.touch_max_y); //touch_max_y
        p_str += size;
    }

    size = write(fd, buf, p_str - buf);
    if (size == (p_str - buf))
    {
        rt_kprintf("file write succeed:\r\n");
    }

    close(fd);
    rt_free(buf);

    return RT_EOK;
}


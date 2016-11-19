#ifndef __TOUCH_H__
#define __TOUCH_H__


void TouchCalibrate(void);
void TP_Config(void) ;
rt_uint16_t TPReadX(void);
rt_uint16_t TPReadY(void);
void xpt2046_write(unsigned char data);

void Touch_ThreadEntry(void* parameter);

#endif


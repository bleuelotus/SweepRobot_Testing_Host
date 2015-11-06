#ifndef __DELAY_H
#define __DELAY_H 			   

#include "sys.h"

#define delay_ms(msec)         delay_us(msec*1000)

void delayinit(u32 clk);
void delay_us(u32 nus);

#endif






























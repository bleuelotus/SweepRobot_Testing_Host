#include "delay.h"

#define  DWT_CR      *(volatile u32 *)0xE0001000
#define  DWT_CYCCNT  *(volatile u32 *)0xE0001004
#define  DEM_CR      *(volatile u32 *)0xE000EDFC
#define  DEM_CR_TRCENA                   (1 << 24)
#define  DWT_CR_CYCCNTENA                (1 <<  0)

static u32 cpuclkfeq;

void delayinit(u32 clk)
{
  cpuclkfeq = clk;

  DEM_CR         |=  DEM_CR_TRCENA; 
  //    DWT_CYCCNT      = 0u;
  DWT_CR         |= DWT_CR_CYCCNTENA;
}

void delay_us(u32 usec)
{
  u32 startts,endts,ts;

  startts = DWT_CYCCNT;
  ts =  usec * (cpuclkfeq /(1000*1000) );
  endts = startts + ts;
  if(endts > startts)
  {
      while(DWT_CYCCNT < endts);
   }
   else
  {
       while(DWT_CYCCNT > endts);
       while(DWT_CYCCNT < endts);
  }
}

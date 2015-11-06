#include "delay.h"
#include "sys.h"

#if SYSTEM_SUPPORT_UCOS
#include "includes.h"
#endif
 
static u8  fac_us=0;
static u16 fac_ms=0;

#ifdef OS_CRITICAL_METHOD 

void SysTick_Handler(void)
{				   
  OSIntEnter();
  OSTimeTick();            
  OSIntExit();
}
#endif

void delayinit(u8 SYSCLK)
{
#ifdef OS_CRITICAL_METHOD
	u32 reload;
#endif
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	 
	fac_us=SYSCLK/8;
	    
#ifdef OS_CRITICAL_METHOD
	reload=SYSCLK/8;
	reload*=1000000/OS_TICKS_PER_SEC;
	fac_ms=1000/OS_TICKS_PER_SEC;
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;
	SysTick->LOAD=reload;
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
#else
	fac_ms=(u16)fac_us*1000;
#endif
}								    

#ifdef OS_CRITICAL_METHOD
								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;
	if(nus==0)return;
	ticks=nus*fac_us;
	tcnt=0;
	OSSchedLock();
	told=SysTick->VAL;
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;
		}  
	};
	OSSchedUnlock();		    
}  

void delay_ms(u16 nms)
{	
	if(OSRunning==OS_TRUE&&OSLockNesting==0)
	{		 
		if(nms>=fac_ms)
		{
   			OSTimeDly(nms/fac_ms);
		}
		nms%=fac_ms;
	}
	delay_us((u32)(nms*1000));
}
#else

void delay_us(u32 nus)
{		
	u32 temp;	    	 
	if(nus==0)return;
	SysTick->LOAD=nus*fac_us;
	SysTick->VAL=0x00;
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL =0X00;
}

void delay_xms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;
	SysTick->VAL =0x00;
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL =0X00;
} 

void delay_ms(u16 nms)
{	 	 
	u8 repeat=nms/540;

	u16 remain=nms%540;
	while(repeat)
	{
		delay_xms(540);
		repeat--;
	}
	if(remain)delay_xms(remain);
} 
#endif
			 

#include"defines.h"
#include "types.h"
void delay_us(u32 tdly)
{
	tdly*=12;
	while(tdly--);
}
void delay_ms(u32 tdly)
{
	tdly*=12000;
	while(tdly--);
}
void delay_s(u32 tdly)
{
	tdly*=12000000;
	while(tdly--);
}

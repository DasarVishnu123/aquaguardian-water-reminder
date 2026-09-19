//LCD.c
#include"types.h"
#include "LCD.h"
#include<lpc21xx.h>
#include "LCD_defines.h"
#include "delay.h"
#include "MACROS.h"
void writeLCD(u8 byte)
{
    WRITEBYTE(IOPIN0,LCD_DATA, byte);

    IOCLR0 = 1 << LCD_RW;

    IOSET0 = 1 << LCD_EN;
    delay_us(1);
    IOCLR0 = 1 << LCD_EN;
    delay_ms(2);
}
void CmdLCD(u8 Cmd)
{
	//select cmd register
	IOCLR0=1<<LCD_RS;
	//write to cmd register
	writeLCD(Cmd);
}
void InitLCD(void)
{
	IODIR0 |=((0xff<<LCD_DATA)| (1<<LCD_RS) | (1<<LCD_RW) | (1<<LCD_EN));
	delay_ms(15);
	CmdLCD(0x30);
	delay_ms(4);
	delay_ms(100);
	CmdLCD(0x30);
	delay_us(100);
	CmdLCD(MODE_8BIT_2LINE);
	CmdLCD(DSP_ON_CUR_OFF);
	CmdLCD(CLEAR_LCD);
	CmdLCD(SHIFT_CUR_RIGHT);
}
void charLCD(u8 asciival)
{
	IOSET0 =1<<LCD_RS;
	writeLCD(asciival);
}	
void strLCD(s8* str)
{
	while(*str)
	charLCD(*str++);
}
void U32LCD(u32 n)
{
	u8 a[10];
	s32 i=0;
	if (n==0)
	{
		charLCD('0');
	}
	else
	{
		while(n>0)
		{
			a[i]=(n%10)+48;
			n/=10;
			i++;
		}
		for(--i;i>=0;i--)
		{
			charLCD(a[i]);
		}
	}
}
void S32LCD(s32 n)
{
	if(n<0)
	{
		charLCD('-');
		n=-n;
	}
	U32LCD(n);
}
void F32LCD(f32 fn,u8 ndp)
{
	u32 n;s32 i;
	if(fn<0.0)
	{
		charLCD('-');
		fn=-fn;
	}
	n=fn;
	U32LCD(n);
	charLCD('.');
	for(i=0;i<ndp;i++)
	{
		fn=(fn-n)*10;
		n=fn;
		charLCD(n+48);
	}
}
void BuildCGRAM(u8 *pattern,u8 nBytes)
{
	u8 i;
	 //pattern &= 0x07;
	CmdLCD(GOTO_CGRAM_START);
	IOSET0=1<<LCD_RS;
	for(i=0;i<nBytes;i++)
	{
		writeLCD(pattern[i]);
	}
	CmdLCD(GOTO_LINE1_POS0);
}


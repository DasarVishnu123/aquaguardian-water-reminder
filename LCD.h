//LCD.h
#include"types.h"
void writeLCD(u8 byte);
void CmdLCD(u8 Cmd);
void InitLCD(void);
void charLCD(u8 ch);
void strLCD(s8 *str);
void U32LCD(u32 n);
void S32LCD(s32 n);
void F32LCD(f32 fn, u8 ndp);
void BuildCGRAM(u8 *pattern, u8 nBytes);

#include <lpc21xx.h>

#include "types.h"
#include "RTC.h"
#include "LCD.h"
#include "LCD_defines.h"


/* Array to hold names of days of the week */
char week[][4] =
{
    "S",
    "M",
    "T",
    "W",
    "T",
    "F",
    "S"
};


/*
Initialize the Real-Time Clock (RTC)

This function disables the RTC,
sets the prescaler values,
and then enables the RTC.
*/
void RTC_Init(void)
{
    /* Disable and reset RTC */
    CCR = RTC_RESET;

    /*
    Set prescaler integer and fractional parts
    */
    PREINT = PREINT_VAL;
    PREFRAC = PREFRAC_VAL;

    /* Enable RTC */
    CCR = RTC_ENABLE;
}


/*
Get the current RTC time
*/
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second)
{
    *hour = HOUR;
    *minute = MIN;
    *second = SEC;
}


/*
Display the RTC time on LCD

Format:

HH:MM:SS
*/
void DisplayRTCTime(u32 hour, u32 minute, u32 second)
{
    CmdLCD(GOTO_LINE1_POS0);

    charLCD(hour/10 + 48);
    charLCD(hour%10 + 48);

    charLCD(':');

    charLCD(minute/10 + 48);
    charLCD(minute%10 + 48);

    charLCD(':');

    charLCD(second/10 + 48);
    charLCD(second%10 + 48);
}


/*
Get the current RTC date
*/
void GetRTCDateInfo(s32 *date, s32 *month, s32 *year)
{
    *date = DOM;
    *month = MONTH;
    *year = YEAR;
}


/*
Display the RTC date on LCD

Format:

DD/MM/YYYY
*/
void DisplayRTCDate(u32 date, u32 month, u32 year)
{
    CmdLCD(GOTO_LINE2_POS0);

    charLCD(date/10 + 48);
    charLCD(date%10 + 48);

    charLCD('/');

    charLCD(month/10 + 48);
    charLCD(month%10 + 48);

    charLCD('/');

    U32LCD(year);
}


/*
Set the RTC time

hour   : 0-23
minute : 0-59
second : 0-59
*/
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second)
{
    HOUR = hour;
    MIN = minute;
    SEC = second;
}


/*
Set the RTC date

date  : 1-31
month : 1-12
year  : four digit year
*/
void SetRTCDateInfo(u32 date, u32 month, u32 year)
{
    DOM = date;
    MONTH = month;
    YEAR = year;
}


/*
Get the current day of the week

0 = Sunday
1 = Monday
2 = Tuesday
...
6 = Saturday
*/
void GetRTCDay(s32 *dow)
{
    *dow = DOW;
}


/*
Display the current day of the week on LCD
*/
void DisplayRTCDay(u32 day)
{
    CmdLCD(GOTO_LINE1_POS0 + 15);

    strLCD(week[day]);
}


/*
Set the day of the week
*/
void SetRTCDay(u32 dow)
{
    DOW = dow;
}

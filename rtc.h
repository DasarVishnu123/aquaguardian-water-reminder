#ifndef RTC_H
#define RTC_H

#include <lpc21xx.h>
#include "types.h"

/* System clock and peripheral clock Macros */
#define FOSC       12000000
#define CCLK       (5*FOSC)
#define PCLK       (CCLK/4)

/* RTC Prescaler Calculation */
#define PREINT_VAL  ((int)(PCLK/32768)-1)
#define PREFRAC_VAL (PCLK-((PREINT_VAL+1)*32768))

/* RTC Control Register Bit Definitions */
#define RTC_ENABLE  (1<<0)
#define RTC_RESET   (1<<1)

/* For LPC2148 */
#define RTC_CLKSRC  (1<<4)

/* Day of week */
#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6

/* Function prototypes */

void RTC_Init(void);

void GetRTCTimeInfo(s32 *, s32 *, s32 *);
void DisplayRTCTime(u32, u32, u32);
void SetRTCTimeInfo(u32, u32, u32);

void GetRTCDateInfo(s32 *, s32 *, s32 *);
void DisplayRTCDate(u32, u32, u32);
void SetRTCDateInfo(u32, u32, u32);

void GetRTCDay(s32 *);
void DisplayRTCDay(u32);
void SetRTCDay(u32);

#endif

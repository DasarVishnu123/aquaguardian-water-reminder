#include <lpc21xx.h>

#include "types.h"
#include "config.h"
#include "LCD.h"
#include "LCD_defines.h"
#include "RTC.h"
#include "KPM.h"
#include "MACROS.h"
#include "delay.h"


/* =========================================================
                        GLOBAL VARIABLES
   ========================================================= */

u32 water_goal       = WATER_GOAL;
u32 drunk            = INITIAL_DRUNK;
u32 reminder_minutes = REMINDER_MINUTES;


/* =========================================================
                        DAILY STATUS
   ========================================================= */

u32 consecutive_neglects = 0;
u32 reminder_active      = 0;
u32 red_alarm            = 0;
u32 goal_reached         = 0;


/* =========================================================
                    REMINDER TIMING
   ========================================================= */

u32 next_reminder  = 0;
u32 reminder_start = 0;


/* =========================================================
                CONFIGURATION INTERRUPT
   ========================================================= */

volatile u32 configuration_request = 0;


/* =========================================================
                TEMPORARY CONFIGURATION
   ========================================================= */

u32 temp_hour;
u32 temp_min;
u32 temp_sec;

u32 temp_date;
u32 temp_month;
u32 temp_year;

u32 temp_reminder;
u32 temp_goal;


/* =========================================================
              10-SECOND DISPLAY REFERENCE
   ========================================================= */

/*
   This time is used ONLY for the LCD second-line
   10-second display cycle.

   Example:

   Reference = 10:00:00

   10:00:00 - 10:00:09
       G:10 D:2 H:20%

   10:00:10 - 10:00:19
       N:10:20 M:0

   10:00:20 - 10:00:29
       G:10 D:2 H:20%

   10:00:30 - 10:00:39
       N:10:40 M:1

   IMPORTANT:

   Pressing drink switch DOES NOT change this time.
*/

u32 display_reference_time = 0;


/* =========================================================
                    CGRAM GLASS CHARACTERS
   ========================================================= */

/* ---------------- EMPTY GLASS ---------------- */

unsigned char empty_glass[8] =
{
    0x0E,
    0x11,
    0x11,
    0x11,
    0x11,
    0x11,
    0x1F,
    0x00
};


/* ---------------- FILLED GLASS ---------------- */

unsigned char filled_glass[8] =
{
    0x0E,
    0x11,
    0x11,
    0x1F,
    0x1F,
    0x1F,
    0x1F,
    0x00
};


/* =========================================================
                    FUNCTION PROTOTYPES
   ========================================================= */

void InitSystem(void);
void InitGPIO(void);
void InitSystemRTC(void);
void InitEINT1(void);
void InitGlassCGRAM(void);

__irq void EINT1_ISR(void);

u32 GetCurrentSeconds(void);
u32 AddSeconds(u32 time, u32 seconds);
u32 GetElapsedSeconds(u32 current, u32 start);

void DisplayNormalScreen(void);

void CheckDrinkSwitch(void);

void ScheduleNextReminder(void);
void StartReminder(void);
void StopReminder(void);
void CheckReminder(void);
void CompleteNeglect(void);

void UpdateLEDs(void);
void UpdateBuzzer(void);

void ConfigurationMode(void);
void ConfigurationMenu(void);

void RTCConfiguration(void);
void TimeConfiguration(void);
void DateConfiguration(void);

void ReminderConfiguration(void);
void GoalConfiguration(void);

u32 ReadConfigValue
(
    s8 *title,
    u32 old_value,
    u32 min_value,
    u32 max_value
);

void ApplyTemporarySettings(void);

void DisplayInvalidInput(void);
void DisplayUpdateConfirmation(void);
void DisplayMessage(s8 *msg);


/* ---------------- Glass Functions ---------------- */

void DisplayGlassWindow(u32 start_index);
void ShowGlassAnimation(void);


/* =========================================================
                    EINT1 INITIALIZATION
   ========================================================= */

void InitEINT1(void)
{
    /*
       P0.3 -> EINT1

       PINSEL0 bits 6 and 7
    */

    PINSEL0 &= ~(3 << 6);
    PINSEL0 |=  (3 << 6);


    /*
       EINT1 edge sensitive
       Falling edge
    */

    EXTMODE |=  (1 << 1);
    EXTPOLAR &= ~(1 << 1);


    /*
       EINT1 -> VIC channel 15
    */

    VICIntSelect &= ~(1 << 15);

    VICVectAddr1 = (unsigned long)EINT1_ISR;

    VICVectCntl1 = (1 << 5) | 15;

    VICIntEnable |= (1 << 15);
}


/* =========================================================
                        EINT1 ISR
   ========================================================= */

__irq void EINT1_ISR(void)
{
    /*
       Only set configuration request flag.
    */

    configuration_request = 1;


    /*
       Clear EINT1 interrupt.
    */

    EXTINT |= (1 << 1);


    /*
       End interrupt.
    */

    VICVectAddr = 0;
}


/* =========================================================
                            MAIN
   ========================================================= */

int main(void)
{
    InitSystem();


    while(1)
    {
        /*
           -------------------------------------------------
           CONFIGURATION REQUEST
           -------------------------------------------------
        */

        if(configuration_request)
        {
            configuration_request = 0;

            ConfigurationMode();
        }


        /*
           -------------------------------------------------
           DRINK SWITCH
           -------------------------------------------------
        */

        CheckDrinkSwitch();


        /*
           -------------------------------------------------
           REMINDER
           -------------------------------------------------
        */

        CheckReminder();


        /*
           -------------------------------------------------
           LED CONTROL
           -------------------------------------------------
        */

        UpdateLEDs();


        /*
           -------------------------------------------------
           BUZZER CONTROL
           -------------------------------------------------
        */

        UpdateBuzzer();


        /*
           -------------------------------------------------
           LCD DISPLAY
           -------------------------------------------------
        */

        DisplayNormalScreen();


        delay_ms(10);
    }
}


/* =========================================================
                    SYSTEM INITIALIZATION
   ========================================================= */

void InitSystem(void)
{
    u32 i;
    u32 j;


    InitGPIO();

    InitLCD();

    Init_KPM();

    InitSystemRTC();

    InitEINT1();

    InitGlassCGRAM();


    /*
       Initial values.
    */

    water_goal       = WATER_GOAL;
    drunk            = INITIAL_DRUNK;
    reminder_minutes = REMINDER_MINUTES;

    consecutive_neglects = 0;
    reminder_active      = 0;
    red_alarm            = 0;
    goal_reached         = 0;


    /*
       Save the original time.

       This controls the 10-second LCD cycle.
    */

    display_reference_time =
        GetCurrentSeconds();


    /*
       Schedule first reminder.
    */

    ScheduleNextReminder();


    /*
       -----------------------------------------------------
       STARTUP MESSAGE
       -----------------------------------------------------
    */

    {
        s8 title[] =
        {
            "                  "
            "AQUAGUARDIAN WATER REMINDER SYSTEM"
            "                  "
        };


        for(i = 0; i < 40; i++)
        {
            CmdLCD(CLEAR_LCD);

            CmdLCD(GOTO_LINE1_POS0);


            for(j = 0; j < 16; j++)
            {
                charLCD(title[i + j]);
            }


            delay_ms(200);
        }
    }


    CmdLCD(CLEAR_LCD);
}


/* =========================================================
                            GPIO
   ========================================================= */

void InitGPIO(void)
{
    /*
       P0.0 -> DRINK SWITCH
       P0.3 -> CONFIGURATION SWITCH


       P1.16 -> BUZZER
       P1.17 -> YELLOW LED
       P1.18 -> RED LED
       P1.19 -> GREEN LED
    */


    IODIR1 |=
        (1 << BUZZER)     |
        (1 << YELLOW_LED) |
        (1 << RED_LED)    |
        (1 << GREEN_LED);


    /*
       Initially all OFF.
    */

    IOCLR1 =
        (1 << BUZZER)     |
        (1 << YELLOW_LED) |
        (1 << RED_LED)    |
        (1 << GREEN_LED);
}


/* =========================================================
                    RTC INITIALIZATION
   ========================================================= */

void InitSystemRTC(void)
{
    RTC_Init();


    /*
       Set initial TIME.
    */

    SetRTCTimeInfo
    (
        START_HOUR,
        START_MIN,
        START_SEC
    );


    /*
       Set initial DATE.

       No DAY configuration.
    */

    SetRTCDateInfo
    (
        START_DATE,
        START_MONTH,
        START_YEAR
    );
}


/* =========================================================
                    INITIALIZE CGRAM
   ========================================================= */

void InitGlassCGRAM(void)
{
    u32 i;


    /*
       CGRAM location 0
       EMPTY GLASS
    */

    CmdLCD(0x40);


    for(i = 0; i < 8; i++)
    {
        charLCD(empty_glass[i]);
    }


    /*
       CGRAM location 1
       FILLED GLASS
    */

    CmdLCD(0x48);


    for(i = 0; i < 8; i++)
    {
        charLCD(filled_glass[i]);
    }


    /*
       Return to DDRAM.
    */

    CmdLCD(0x80);
}


/* =========================================================
                    GET CURRENT SECONDS
   ========================================================= */

u32 GetCurrentSeconds(void)
{
    s32 hour;
    s32 minute;
    s32 second;


    GetRTCTimeInfo
    (
        &hour,
        &minute,
        &second
    );


    return
        ((u32)hour * SECONDS_PER_HOUR) +
        ((u32)minute * SECONDS_PER_MINUTE) +
        (u32)second;
}


/* =========================================================
                        ADD SECONDS
   ========================================================= */

u32 AddSeconds(u32 time, u32 seconds)
{
    time += seconds;


    if(time >= SECONDS_PER_DAY)
    {
        time %= SECONDS_PER_DAY;
    }


    return time;
}


/* =========================================================
                    GET ELAPSED SECONDS
   ========================================================= */

u32 GetElapsedSeconds(u32 current, u32 start)
{
    if(current >= start)
    {
        return current - start;
    }


    /*
       Handle time rollover.
    */

    return
        (SECONDS_PER_DAY - start) +
        current;
}


/* =========================================================
                    NORMAL LCD SCREEN
   ========================================================= */

void DisplayNormalScreen(void)
{
    s32 hour;
    s32 minute;
    s32 second;

    s32 date;
    s32 month;
    s32 year;

    u32 hydration;

    u32 current;
    u32 elapsed_from_start;
    u32 display_mode;

    u32 next_hour;
    u32 next_minute;


    /*
       -----------------------------------------------------
       GET RTC TIME
       -----------------------------------------------------
    */

    GetRTCTimeInfo
    (
        &hour,
        &minute,
        &second
    );


    /*
       -----------------------------------------------------
       GET RTC DATE
       -----------------------------------------------------
    */

    GetRTCDateInfo
    (
        &date,
        &month,
        &year
    );


    /*
       -----------------------------------------------------
       HYDRATION PERCENTAGE
       -----------------------------------------------------
    */

    if(water_goal != 0)
    {
        hydration =
            (drunk * 100) / water_goal;
    }
    else
    {
        hydration = 0;
    }


    /*
       -----------------------------------------------------
       CURRENT TIME IN SECONDS
       -----------------------------------------------------
    */

    current =
        GetCurrentSeconds();


    /*
       =====================================================
                            LINE 1
       =====================================================

       HH:MM:SS DD/MM
    */

    CmdLCD(GOTO_LINE1_POS0);


    charLCD((hour / 10) + '0');
    charLCD((hour % 10) + '0');

    charLCD(':');

    charLCD((minute / 10) + '0');
    charLCD((minute % 10) + '0');

    charLCD(':');

    charLCD((second / 10) + '0');
    charLCD((second % 10) + '0');

    charLCD(' ');

    charLCD((date / 10) + '0');
    charLCD((date % 10) + '0');

    charLCD('/');

    charLCD((month / 10) + '0');
    charLCD((month % 10) + '0');

    charLCD(' ');
    charLCD(' ');
    charLCD(' ');


    /*
       =====================================================
                            LINE 2
       =====================================================
    */

    CmdLCD(GOTO_LINE2_POS0);


    /*
       -----------------------------------------------------
       REMINDER ACTIVE
       -----------------------------------------------------

       During reminder:

       Yellow LED = ON
       Buzzer     = ON
    */

    if(reminder_active)
    {
        strLCD("   DRINK WATER  ");

        return;
    }


    /*
       -----------------------------------------------------
       GOAL REACHED
       -----------------------------------------------------

       IMPORTANT:

       Only when:

           D == G

       Line 2 only:

           GOAL REACHED
    */

    if(drunk == water_goal)
    {
        strLCD("  GOAL REACHED  ");

        return;
    }


    /*
       -----------------------------------------------------
       10-SECOND DISPLAY CYCLE
       -----------------------------------------------------

       Based on ORIGINAL reference time.

       Drink switch does NOT reset this timing.
    */

    elapsed_from_start =
        GetElapsedSeconds
        (
            current,
            display_reference_time
        );


    /*
       Every 10 seconds the display changes.

       0 -> G/D/H
       1 -> NEXT REMINDER + MISSED
       2 -> G/D/H
       3 -> NEXT REMINDER + MISSED
       ...
    */

    display_mode =
        (elapsed_from_start / 10) % 2;


    /*
       =====================================================
       MODE 0

       G:10 D:2 H:20%
       =====================================================
    */

    if(display_mode == 0)
    {
        charLCD('G');
        charLCD(':');

        U32LCD(water_goal);

        charLCD(' ');

        charLCD('D');
        charLCD(':');

        U32LCD(drunk);

        charLCD(' ');

        charLCD('H');
        charLCD(':');

        U32LCD(hydration);

        charLCD('%');

        charLCD(' ');
        charLCD(' ');
        charLCD(' ');
    }


    /*
       =====================================================
       MODE 1

       N:10:20 M:1
       =====================================================
    */

    else
    {
        /*
           Convert next reminder seconds
           into HH:MM.
        */

        next_hour =
            next_reminder / SECONDS_PER_HOUR;


        next_minute =
            (next_reminder % SECONDS_PER_HOUR)
            / SECONDS_PER_MINUTE;


        /*
           N:HH:MM
        */

        charLCD('N');
        charLCD(':');

        charLCD((next_hour / 10) + '0');
        charLCD((next_hour % 10) + '0');

        charLCD(':');

        charLCD((next_minute / 10) + '0');
        charLCD((next_minute % 10) + '0');


        charLCD(' ');


        /*
           M = missed count.
        */

        charLCD('M');
        charLCD(':');

        U32LCD(consecutive_neglects);


        /*
           Clear remaining LCD positions.
        */

        charLCD(' ');
        charLCD(' ');
        charLCD(' ');
        charLCD(' ');
        charLCD(' ');
        charLCD(' ');
        charLCD(' ');
    }
}


/* =========================================================
                    CHECK DRINK SWITCH
   ========================================================= */

void CheckDrinkSwitch(void)
{
    /*
       Active LOW drink switch.
    */

    if((IOPIN0 & (1 << DRINK_SW)) == 0)
    {
        /*
           Debounce.
        */

        delay_ms(30);


        if((IOPIN0 & (1 << DRINK_SW)) == 0)
        {
            /*
               Drink switch works only during reminder.
            */

            if(reminder_active)
            {
                /*
                   Increase drunk count.
                */

                drunk++;


                /*
                   ------------------------------------------------
                   IMMEDIATELY TURN OFF:

                   Yellow LED
                   Buzzer
                   ------------------------------------------------
                */

                reminder_active = 0;


                IOCLR1 =
                    (1 << YELLOW_LED) |
                    (1 << BUZZER);


                /*
                   ------------------------------------------------
                   CHECK GOAL

                   D == G
                   ------------------------------------------------
                */

                if(drunk >= water_goal)
                {
                    drunk = water_goal;

                    goal_reached = 1;

                    consecutive_neglects = 0;

                    red_alarm = 0;


                    /*
                       Show final glass status.
                    */

                    ShowGlassAnimation();
                }


                /*
                   ------------------------------------------------
                   GOAL NOT REACHED
                   ------------------------------------------------
                */

                else
                {
                    /*
                       Successful drinking.

                       Reset missed count.
                    */

                    consecutive_neglects = 0;


                    /*
                       Schedule next reminder from the
                       previous reminder schedule.

                       Do NOT change display_reference_time.
                    */

                    next_reminder =
                        AddSeconds
                        (
                            next_reminder,
                            reminder_minutes *
                            SECONDS_PER_MINUTE
                        );


                    /*
                       Show water status.

                       Buzzer and yellow LED remain OFF.
                    */

                    ShowGlassAnimation();
                }
            }


            /*
               Wait for switch release.
            */

            while((IOPIN0 & (1 << DRINK_SW)) == 0)
            {
            }


            delay_ms(30);
        }
    }
}


/* =========================================================
                    SCHEDULE NEXT REMINDER
   ========================================================= */

void ScheduleNextReminder(void)
{
    next_reminder =
        AddSeconds
        (
            GetCurrentSeconds(),
            reminder_minutes *
            SECONDS_PER_MINUTE
        );
}


/* =========================================================
                        START REMINDER
   ========================================================= */

void StartReminder(void)
{
    /*
       Do not start reminder when D == G.
    */

    if(drunk == water_goal)
    {
        goal_reached = 1;

        reminder_active = 0;

        return;
    }


    /*
       Start reminder.
    */

    reminder_active = 1;

    reminder_start =
        GetCurrentSeconds();
}


/* =========================================================
                        STOP REMINDER
   ========================================================= */

void StopReminder(void)
{
    reminder_active = 0;


    /*
       Immediately turn OFF:

       Yellow LED
       Buzzer
    */

    IOCLR1 =
        (1 << YELLOW_LED) |
        (1 << BUZZER);
}


/* =========================================================
                        CHECK REMINDER
   ========================================================= */

void CheckReminder(void)
{
    u32 current;
    u32 elapsed;


    /*
       Do not remind after D == G.
    */

    if(drunk == water_goal)
    {
        reminder_active = 0;

        return;
    }


    current =
        GetCurrentSeconds();


    /*
       -----------------------------------------------------
       WAIT FOR NEXT REMINDER
       -----------------------------------------------------
    */

    if(reminder_active == 0)
    {
        if(current == next_reminder)
        {
            StartReminder();
        }


        return;
    }


    /*
       -----------------------------------------------------
       REMINDER ACTIVE
       -----------------------------------------------------
    */

    elapsed =
        GetElapsedSeconds
        (
            current,
            reminder_start
        );


    /*
       Reminder duration completed.
    */

    if(elapsed >= REMINDER_DURATION_SEC)
    {
        reminder_active = 0;


        /*
           Yellow LED and buzzer OFF.
        */

        IOCLR1 =
            (1 << YELLOW_LED) |
            (1 << BUZZER);


        /*
           Count missed reminder.
        */

        CompleteNeglect();


        /*
           Schedule next reminder from previous
           reminder schedule.
        */

        next_reminder =
            AddSeconds
            (
                next_reminder,
                reminder_minutes *
                SECONDS_PER_MINUTE
            );
    }
}


/* =========================================================
                    COMPLETE NEGLECT
   ========================================================= */

void CompleteNeglect(void)
{
    consecutive_neglects++;


    if(consecutive_neglects >= MAX_NEGLECTS)
    {
        red_alarm = 1;
    }
}


/* =========================================================
                        UPDATE LEDs
   ========================================================= */

void UpdateLEDs(void)
{
    /*
       -----------------------------------------------------
       YELLOW LED
       -----------------------------------------------------
    */

    if(reminder_active && (drunk != water_goal))
    {
        IOSET1 = (1 << YELLOW_LED);
    }
    else
    {
        IOCLR1 = (1 << YELLOW_LED);
    }


    /*
       -----------------------------------------------------
       RED LED
       -----------------------------------------------------
    */

    if(red_alarm)
    {
        IOSET1 = (1 << RED_LED);
    }
    else
    {
        IOCLR1 = (1 << RED_LED);
    }


    /*
       -----------------------------------------------------
       GREEN LED
       -----------------------------------------------------

       Green ON only when:

           D == G
    */

    if(drunk == water_goal)
    {
        IOSET1 = (1 << GREEN_LED);
    }
    else
    {
        IOCLR1 = (1 << GREEN_LED);
    }
}


/* =========================================================
                        UPDATE BUZZER
   ========================================================= */

void UpdateBuzzer(void)
{
    /*
       Buzzer ON only during reminder.
    */

    if(reminder_active && (drunk != water_goal))
    {
        IOSET1 = (1 << BUZZER);
    }
    else
    {
        IOCLR1 = (1 << BUZZER);
    }
}


/* =========================================================
                    DISPLAY GLASS WINDOW
   ========================================================= */

void DisplayGlassWindow(u32 start_index)
{
    u32 i;
    u32 glass_index;


    CmdLCD(GOTO_LINE2_POS0);


    /*
       Display exactly 16 LCD positions.
    */

    for(i = 0; i < 16; i++)
    {
        glass_index =
            start_index + i;


        /*
           Glass is inside goal.
        */

        if(glass_index < water_goal)
        {
            /*
               Filled glass.
            */

            if(glass_index < drunk)
            {
                charLCD(1);
            }


            /*
               Empty glass.
            */

            else
            {
                charLCD(0);
            }
        }


        /*
           No glass beyond goal.
        */

        else
        {
            charLCD(' ');
        }
    }
}


/* =========================================================
                    SHOW GLASS ANIMATION
   ========================================================= */

void ShowGlassAnimation(void)
{
    u32 start_index;
    u32 max_start;
    u32 i;


    /*
       During water status:

       Yellow LED = OFF
       Buzzer     = OFF
    */

    IOCLR1 =
        (1 << YELLOW_LED) |
        (1 << BUZZER);


    /*
       -----------------------------------------------------
       GOAL <= 16
       -----------------------------------------------------
    */

    if(water_goal <= 16)
    {
        CmdLCD(CLEAR_LCD);


        CmdLCD(GOTO_LINE1_POS0);
        strLCD(" WATER STATUS   ");


        DisplayGlassWindow(0);


        delay_ms(1500);


        /*
           Keep outputs OFF.
        */

        IOCLR1 =
            (1 << YELLOW_LED) |
            (1 << BUZZER);


        return;
    }


    /*
       -----------------------------------------------------
       GOAL > 16

       Example:

       Goal = 21

       Window 1 -> Glass 1  to Glass 16
       Window 2 -> Glass 2  to Glass 17
       Window 3 -> Glass 3  to Glass 18
       Window 4 -> Glass 4  to Glass 19
       Window 5 -> Glass 5  to Glass 20
       Window 6 -> Glass 6  to Glass 21
       -----------------------------------------------------
    */

    max_start =
        water_goal - 16;


    for(i = 0; i <= max_start; i++)
    {
        start_index = i;


        /*
           Keep buzzer and yellow LED OFF.
        */

        IOCLR1 =
            (1 << YELLOW_LED) |
            (1 << BUZZER);


        CmdLCD(CLEAR_LCD);


        CmdLCD(GOTO_LINE1_POS0);
        strLCD(" WATER STATUS   ");


        DisplayGlassWindow(start_index);


        delay_ms(350);
    }


    /*
       Keep final window visible.
    */

    delay_ms(700);


    /*
       Keep outputs OFF.
    */

    IOCLR1 =
        (1 << YELLOW_LED) |
        (1 << BUZZER);
}


/* =========================================================
                    CONFIGURATION MODE
   ========================================================= */

void ConfigurationMode(void)
{
    /*
       Read current TIME.
    */

    GetRTCTimeInfo
    (
        (s32 *)&temp_hour,
        (s32 *)&temp_min,
        (s32 *)&temp_sec
    );


    /*
       Read current DATE.
    */

    GetRTCDateInfo
    (
        (s32 *)&temp_date,
        (s32 *)&temp_month,
        (s32 *)&temp_year
    );


    /*
       Save current REMINDER and GOAL.
    */

    temp_reminder = reminder_minutes;

    temp_goal = water_goal;


    /*
       Stop reminder.
    */

    StopReminder();


    ConfigurationMenu();
}


/* =========================================================
                    CONFIGURATION MENU
   ========================================================= */

void ConfigurationMenu(void)
{
    u32 key;
    u32 exit_menu = 0;


    while(exit_menu == 0)
    {
        CmdLCD(CLEAR_LCD);


        /*
           -------------------------------------------------
           ONLY FOUR OPTIONS
           -------------------------------------------------

           1 -> RTC
           2 -> GOAL
           3 -> REMINDER
           4 -> EXIT
        */

        CmdLCD(GOTO_LINE1_POS0);
        strLCD("1.RTC 2.GOAL");


        CmdLCD(GOTO_LINE2_POS0);
        strLCD("3.REM 4.EXIT");


        key = keyscan();


        /*
           -------------------------------------------------
           RTC
           -------------------------------------------------
        */

        if(key == '1')
        {
            RTCConfiguration();
        }


        /*
           -------------------------------------------------
           GOAL
           -------------------------------------------------
        */

        else if(key == '2')
        {
            GoalConfiguration();
        }


        /*
           -------------------------------------------------
           REMINDER
           -------------------------------------------------
        */

        else if(key == '3')
        {
            ReminderConfiguration();
        }


        /*
           -------------------------------------------------
           EXIT
           -------------------------------------------------
        */

        else if(key == '4')
        {
            DisplayUpdateConfirmation();


            key = keyscan();


            /*
               = -> YES
            */

            if(key == KEY_ENTER)
            {
                ApplyTemporarySettings();


                DisplayMessage("    UPDATED     ");


                delay_ms(700);


                exit_menu = 1;
            }


            /*
               * -> NO / CANCEL
            */

            else if(key == KEY_CLEAR)
            {
                DisplayMessage("   CANCELLED    ");


                delay_ms(700);


                exit_menu = 1;
            }
        }
    }
}


/* =========================================================
                    RTC CONFIGURATION
   ========================================================= */

void RTCConfiguration(void)
{
    u32 key;


    while(1)
    {
        CmdLCD(CLEAR_LCD);


        /*
           Only TIME and DATE.
        */

        CmdLCD(GOTO_LINE1_POS0);
        strLCD("1.TIME 2.DATE");


        CmdLCD(GOTO_LINE2_POS0);
        strLCD("     *BACK");


        key = keyscan();


        /*
           TIME
        */

        if(key == '1')
        {
            TimeConfiguration();
        }


        /*
           DATE
        */

        else if(key == '2')
        {
            DateConfiguration();
        }


        /*
           BACK
        */

        else if(key == KEY_CLEAR)
        {
            return;
        }
    }
}


/* =========================================================
                    TIME CONFIGURATION
   ========================================================= */

void TimeConfiguration(void)
{
    u32 key;


    while(1)
    {
        CmdLCD(CLEAR_LCD);


        CmdLCD(GOTO_LINE1_POS0);
        strLCD("TIME");


        CmdLCD(GOTO_LINE2_POS0);
        strLCD("1.HH 2.MM 3.SS");


        key = keyscan();


        /*
           HOUR
        */

        if(key == '1')
        {
            temp_hour =
                ReadConfigValue
                (
                    "ENTER HH(0-23)",
                    temp_hour,
                    MIN_HOUR,
                    MAX_HOUR
                );
        }


        /*
           MINUTE
        */

        else if(key == '2')
        {
            temp_min =
                ReadConfigValue
                (
                    "ENTER MM(0-59)",
                    temp_min,
                    MIN_MINUTE,
                    MAX_MINUTE
                );
        }


        /*
           SECOND
        */

        else if(key == '3')
        {
            temp_sec =
                ReadConfigValue
                (
                    "ENTER SS(0-59)",
                    temp_sec,
                    MIN_SECOND,
                    MAX_SECOND
                );
        }


        /*
           BACK
        */

        else if(key == KEY_CLEAR)
        {
            return;
        }
    }
}


/* =========================================================
                    DATE CONFIGURATION
   ========================================================= */

void DateConfiguration(void)
{
    u32 key;


    while(1)
    {
        CmdLCD(CLEAR_LCD);


        CmdLCD(GOTO_LINE1_POS0);
        strLCD("DATE");


        CmdLCD(GOTO_LINE2_POS0);
        strLCD("1.DATE 2.MONTH");


        key = keyscan();


        /*
           DATE
        */

        if(key == '1')
        {
            temp_date =
                ReadConfigValue
                (
                    "ENTER DATE(1-31)",
                    temp_date,
                    MIN_DATE,
                    MAX_DATE
                );
        }


        /*
           MONTH
        */

        else if(key == '2')
        {
            temp_month =
                ReadConfigValue
                (
                    "ENTER MON(1-12)",
                    temp_month,
                    MIN_MONTH,
                    MAX_MONTH
                );
        }


        /*
           BACK
        */

        else if(key == KEY_CLEAR)
        {
            return;
        }
    }
}


/* =========================================================
                    REMINDER CONFIGURATION
   ========================================================= */

void ReminderConfiguration(void)
{
    temp_reminder =
        ReadConfigValue
        (
            "ENTER MIN",
            temp_reminder,
            MIN_REMINDER,
            MAX_REMINDER
        );
}


/* =========================================================
                    GOAL CONFIGURATION
   ========================================================= */

void GoalConfiguration(void)
{
    temp_goal =
        ReadConfigValue
        (
            "ENTER GOAL",
            temp_goal,
            MIN_GOAL,
            MAX_GOAL
        );
}


/* =========================================================
                    READ CONFIGURATION VALUE
   ========================================================= */

u32 ReadConfigValue
(
    s8 *title,
    u32 old_value,
    u32 min_value,
    u32 max_value
)
{
    u32 value = 0;
    u32 digits = 0;
    u32 key;


    CmdLCD(CLEAR_LCD);


    CmdLCD(GOTO_LINE1_POS0);
    strLCD(title);


    CmdLCD(GOTO_LINE2_POS0);


    while(1)
    {
        key = keyscan();


        /*
           '/' -> SKIP
        */

        if(key == KEY_SKIP)
        {
            return old_value;
        }


        /*
           '*' -> CLEAR
        */

        if(key == KEY_CLEAR)
        {
            value = 0;

            digits = 0;


            CmdLCD(GOTO_LINE2_POS0);

            strLCD("                ");


            CmdLCD(GOTO_LINE2_POS0);


            continue;
        }


        /*
           '-' -> BACKSPACE
        */

        if(key == KEY_BACKSPACE)
        {
            if(digits > 0)
            {
                value /= 10;

                digits--;


                CmdLCD(GOTO_LINE2_POS0);

                strLCD("                ");


                CmdLCD(GOTO_LINE2_POS0);


                if(digits > 0)
                {
                    U32LCD(value);
                }
            }


            continue;
        }


        /*
           '=' -> ENTER
        */

        if(key == KEY_ENTER)
        {
            if
            (
                digits > 0 &&
                value >= min_value &&
                value <= max_value
            )
            {
                return value;
            }


            DisplayInvalidInput();


            value = 0;

            digits = 0;


            CmdLCD(CLEAR_LCD);


            CmdLCD(GOTO_LINE1_POS0);
            strLCD(title);


            CmdLCD(GOTO_LINE2_POS0);


            continue;
        }


        /*
           NUMERIC KEYS
        */

        if(key >= '0' && key <= '9')
        {
            if(digits < 4)
            {
                value =
                    (value * 10) +
                    (key - '0');


                digits++;


                CmdLCD(GOTO_LINE2_POS0);

                strLCD("                ");


                CmdLCD(GOTO_LINE2_POS0);

                U32LCD(value);
            }
        }
    }
}


/* =========================================================
                    APPLY TEMPORARY SETTINGS
   ========================================================= */

void ApplyTemporarySettings(void)
{
    /*
       -----------------------------------------------------
       APPLY TIME
       -----------------------------------------------------
    */

    SetRTCTimeInfo
    (
        temp_hour,
        temp_min,
        temp_sec
    );


    /*
       -----------------------------------------------------
       APPLY DATE

       Only DATE and MONTH are configured.

       No DAY configuration.
       -----------------------------------------------------
    */

    SetRTCDateInfo
    (
        temp_date,
        temp_month,
        temp_year
    );


    /*
       -----------------------------------------------------
       APPLY REMINDER
       -----------------------------------------------------
    */

    reminder_minutes =
        temp_reminder;


    /*
       -----------------------------------------------------
       APPLY GOAL
       -----------------------------------------------------
    */

    water_goal =
        temp_goal;


    /*
       -----------------------------------------------------
       RESET CURRENT DAY VALUES
       -----------------------------------------------------
    */

    drunk = 0;

    consecutive_neglects = 0;

    red_alarm = 0;

    reminder_active = 0;

    goal_reached = 0;


    /*
       Buzzer and yellow LED OFF.
    */

    IOCLR1 =
        (1 << YELLOW_LED) |
        (1 << BUZZER);


    /*
       -----------------------------------------------------
       NEW REMINDER SCHEDULE
       -----------------------------------------------------
    */

    ScheduleNextReminder();


    /*
       -----------------------------------------------------
       IMPORTANT

       New configured time becomes the reference
       for the 10-second display cycle.
       -----------------------------------------------------
    */

    display_reference_time =
        GetCurrentSeconds();
}


/* =========================================================
                    INVALID INPUT
   ========================================================= */

void DisplayInvalidInput(void)
{
    CmdLCD(CLEAR_LCD);


    CmdLCD(GOTO_LINE1_POS0);
    strLCD("INVALID INPUT");


    CmdLCD(GOTO_LINE2_POS0);
    strLCD("TRY AGAIN");


    delay_ms(1000);
}


/* =========================================================
                    UPDATE CONFIRMATION
   ========================================================= */

void DisplayUpdateConfirmation(void)
{
    CmdLCD(CLEAR_LCD);


    CmdLCD(GOTO_LINE1_POS0);
    strLCD("UPDATE SETTINGS?");


    CmdLCD(GOTO_LINE2_POS0);
    strLCD("=YES *NO");
}


/* =========================================================
                        DISPLAY MESSAGE
   ========================================================= */

void DisplayMessage(s8 *msg)
{
    CmdLCD(CLEAR_LCD);


    CmdLCD(GOTO_LINE1_POS0);

    strLCD(msg);
}

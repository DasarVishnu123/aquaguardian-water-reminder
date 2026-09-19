#ifndef CONFIG_H
#define CONFIG_H
#include "types.h"

//SWITCHES
#define DRINK_SW        0       //P0.0
#define CONFIG_INT_PIN  3       //P0.3 EINT1


//OUTPUTS
#define BUZZER          16      /* P1.16 */
#define YELLOW_LED      17      /* P1.17 */
#define RED_LED         18      /* P1.18 */
#define GREEN_LED       19      /* P1.19 */

//START RTC 
#define START_HOUR      10
#define START_MIN       0
#define START_SEC       0

#define START_DATE      10
#define START_MONTH     8
#define START_YEAR      2026

//WATER
#define WATER_GOAL              10
#define INITIAL_DRUNK           0
#define REMINDER_MINUTES        2

#define MIN_GOAL                1
#define MAX_GOAL                99

#define MIN_REMINDER            1
#define MAX_REMINDER            999


//RMINDER
#define REMINDER_DURATION_SEC   30
#define MAX_NEGLECTS            3

//TIME LIMITS 
#define MIN_HOUR        0
#define MAX_HOUR        23

#define MIN_MINUTE      0
#define MAX_MINUTE      59

#define MIN_SECOND      0
#define MAX_SECOND      59

//DATE LIMITS
#define MIN_DATE        1
#define MAX_DATE        31

#define MIN_MONTH       1
#define MAX_MONTH       12

#define MIN_YEAR        2000
#define MAX_YEAR        2099


//TIME CONSTANTS 
#define SECONDS_PER_MINUTE    60UL
#define SECONDS_PER_HOUR      3600UL
#define SECONDS_PER_DAY       86400UL


//SPECIAL KEYPAD KEYS
#define KEY_ENTER       '='
#define KEY_CLEAR       '*'
#define KEY_BACKSPACE   '-'
#define KEY_SKIP        '/'

//FUNCTION PROTOTYPES
void InitSystem(void);
void InitGPIO(void);
void InitSystemRTC(void);
void InitEINT1(void);
void InitGlassCGRAM(void);

__irq void EINT1_ISR(void);

u32 GetCurrentSeconds(void);
u32 AddSeconds(u32 time, u32 seconds);
u32 GetElapsedSeconds(u32 current, u32 start);
u32 TimeReached(u32 current, u32 target);

void DisplayNormalScreen(void);
void DisplayNextReminder(void);

void CheckDrinkSwitch(void);

void ScheduleNextReminder(void);
void StartReminder(void);
void StopReminder(void);
void CheckReminder(void);
void CompleteNeglect(void);

void UpdateLEDs(void);
void UpdateBuzzer(void);

void MidnightReset(void);

void ConfigurationMode(void);
void ConfigurationMenu(void);

void RTCConfiguration(void);
void TimeConfiguration(void);
void DateConfiguration(void);
void DayConfiguration(void);

void ReminderConfiguration(void);
void GoalConfiguration(void);

u32 ReadConfigValue(s8 *title,u32 old_value,u32 min_value,u32 max_value);

void ApplyTemporarySettings(void);

void DisplayInvalidInput(void);
void DisplayUpdateConfirmation(void);
void DisplayMessage(s8 *msg);


/* ---------------- Glass Functions ---------------- */

void DisplayGlassWindow(u32 start_index);
void ShowGlassAnimation(void);

#endif

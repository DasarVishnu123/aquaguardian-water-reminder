#include <lpc21xx.h>
#include "types.h"
#include "config.h"
#include "LCD.h"
#include "LCD_defines.h"
#include "RTC.h"
#include "KPM.h"
#include "MACROS.h"
#include "dealy.h"

// Hydration settings
u32 targetGlassCount          = WATER_GOAL;
u32 consumedGlasses           = INITIAL_DRUNK;
u32 reminderIntervalMinutes   = REMINDER_MINUTES;

// Operational status flags
u32 consecutiveMissedReminders = 0;
u32 isReminderAlertActive      = 0;
u32 isCriticalAlarmTriggered   = 0;
u32 isDailyGoalAchieved        = 0;

// Timing counters
u32 nextReminderTimestamp     = 0;
u32 reminderStartTimestamp    = 0;

// Interrupt flags
volatile u32 isConfigMenuRequested = 0;

// Temporary configuration storage
s32 configBufferHour;
s32 configBufferMinute;
s32 configBufferSecond;
s32 configBufferDate;
s32 configBufferMonth;
s32 configBufferYear;
u32 configBufferInterval;
u32 configBufferGoal;

// Display refresh reference
u32 displayCycleBaseTimestamp = 0;

// LCD custom graphics
unsigned char emptyGlassIcon[8]  = { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x00 };
unsigned char filledGlassIcon[8] = { 0x0E, 0x11, 0x11, 0x1F, 0x1F, 0x1F, 0x1F, 0x00 };

// Function prototypes
void InitSystem(void);
void InitGPIO(void);
void InitSystemRTC(void);
void InitEINT1(void);
void InitGlassCGRAM(void);
__irq void EINT1_ISR(void);
u32 GetCurrentSeconds(void);
u32 AddSeconds(u32 baseSeconds, u32 secondsToAdd);
u32 GetElapsedSeconds(u32 currentSeconds, u32 startSeconds);
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
u32 ReadConfigValue(s8 *promptTitle, u32 fallbackValue, u32 allowedMin, u32 allowedMax);
void ApplyTemporarySettings(void);
void DisplayInvalidInput(void);
void DisplayUpdateConfirmation(void);
void DisplayMessage(s8 *statusMessage);
void DisplayGlassWindow(u32 startWindowIndex);
void ShowGlassAnimation(void);

// Configure EINT1 for external interrupt switch
void InitEINT1(void)
{
    PINSEL0 &= ~(3 << 6);                   // Clear P0.3 pin selection bits
    PINSEL0 |=  (3 << 6);                   // Route P0.3 to EINT1
    EXTMODE |=  (1 << 1);                   // Set EINT1 to edge-sensitive mode
    EXTPOLAR &= ~(1 << 1);                  // Set trigger to falling edge
    EXTINT = (1 << 1);                      // Clear any pending interrupt flags
    VICIntSelect &= ~(1 << 15);             // Configure EINT1 channel as IRQ
    VICVectAddr1 = (unsigned long)EINT1_ISR;// Register ISR address in slot 1
    VICVectCntl1 = (1 << 5) | 15;           // Enable slot 1 for VIC channel 15
    VICIntEnable |= (1 << 15);              // Enable EINT1 interrupt source
}

// EINT1 interrupt service routine
__irq void EINT1_ISR(void)
{
    isConfigMenuRequested = 1;              // Request configuration menu entry
    EXTINT = (1 << 1);                      // Clear EINT1 peripheral interrupt flag
    VICVectAddr = 0;                        // Signal interrupt completion to VIC
}

// Main execution entry point
int main(void)
{
    u32 currentSecondCounter;
    static u32 previousSecondCounter = 0xFF; // Track seconds to prevent LCD flicker

    InitSystem();                           // Initialize peripherals and clock

    while(1)
    {
        if(isConfigMenuRequested)           // Check if config switch was pressed
        {
            isConfigMenuRequested = 0;      // Acknowledge interrupt flag
            ConfigurationMode();            // Launch interactive menu
            previousSecondCounter = 0xFF;   // Force immediate LCD redraw
        }

        CheckDrinkSwitch();                 // Poll hydration switch status
        CheckReminder();                    // Evaluate reminder thresholds
        UpdateLEDs();                       // Refresh LED indicators
        UpdateBuzzer();                     // Refresh alarm buzzer status

        currentSecondCounter = GetCurrentSeconds(); // Fetch current timestamp
        if(currentSecondCounter != previousSecondCounter) // Refresh display strictly once per second
        {
            previousSecondCounter = currentSecondCounter; // Cache current second
            DisplayNormalScreen();          // Render main UI without flicker
        }

        delay_ms(10);                       // Maintain base polling loop interval
    }
}

// Initialize system peripherals and initial values
void InitSystem(void)
{
    u32 scrollStepIndex, characterIndex;
    s8 bannerTitle[] = "                AQUAGUARDIAN WATER REMINDER SYSTEM                ";

    InitGPIO();                             // Setup GPIO directions
    InitLCD();                              // Initialize HD44780 LCD module
    Init_KPM();                             // Initialize keypad matrix pins
    InitSystemRTC();                        // Initialize hardware real-time clock
    InitEINT1();                            // Initialize external interrupt
    InitGlassCGRAM();                       // Load glass icons into LCD CGRAM

    targetGlassCount          = WATER_GOAL; // Set default hydration target
    consumedGlasses           = INITIAL_DRUNK; // Clear intake counter
    reminderIntervalMinutes   = REMINDER_MINUTES; // Set default reminder interval

    consecutiveMissedReminders = 0;         // Reset consecutive neglect counter
    isReminderAlertActive      = 0;         // Deactivate active reminder state
    isCriticalAlarmTriggered   = 0;         // Clear neglect warning state
    isDailyGoalAchieved        = 0;         // Clear goal achievement flag

    displayCycleBaseTimestamp = GetCurrentSeconds(); // Lock initial display base time
    ScheduleNextReminder();                 // Calculate first reminder target

    for(scrollStepIndex = 0; scrollStepIndex < 40; scrollStepIndex++) // Scroll marquee title text across screen
    {
        CmdLCD(CLEAR_LCD);                  // Clear display memory
        CmdLCD(GOTO_LINE1_POS0);            // Move cursor to start of line 1
        for(characterIndex = 0; characterIndex < 16; characterIndex++) // Print 16 visible window characters
        {
            charLCD(bannerTitle[scrollStepIndex + characterIndex]); // Render character to display
        }
        delay_ms(200);                      // Delay scrolling shift speed
    }

    CmdLCD(CLEAR_LCD);                      // Clear marquee text after completion
}

// Setup GPIO port directions and default levels
void InitGPIO(void)
{
    IODIR1 |= (1 << BUZZER) | (1 << YELLOW_LED) | (1 << RED_LED) | (1 << GREEN_LED); // Set control outputs
    IOCLR1 =  (1 << BUZZER) | (1 << YELLOW_LED) | (1 << RED_LED) | (1 << GREEN_LED); // Drive all outputs low
}

// Configure RTC with default date and time
void InitSystemRTC(void)
{
    RTC_Init();                             // Start internal RTC oscillator
    SetRTCTimeInfo(START_HOUR, START_MIN, START_SEC); // Write default initial time
    SetRTCDateInfo(START_DATE, START_MONTH, START_YEAR); // Write default initial date
}

// Load custom glass bitmap data into LCD CGRAM
void InitGlassCGRAM(void)
{
    u32 iconRowIndex;

    CmdLCD(0x40);                           // Set CGRAM pointer to address 0
    for(iconRowIndex = 0; iconRowIndex < 8; iconRowIndex++) charLCD(emptyGlassIcon[iconRowIndex]); // Write empty glass glyph

    CmdLCD(0x48);                           // Set CGRAM pointer to address 1
    for(iconRowIndex = 0; iconRowIndex < 8; iconRowIndex++) charLCD(filledGlassIcon[iconRowIndex]); // Write filled glass glyph

    CmdLCD(0x80);                           // Return memory pointer to DDRAM Line 1
}

// Calculate total elapsed seconds from midnight
u32 GetCurrentSeconds(void)
{
    s32 currentHour, currentMinute, currentSecond;

    GetRTCTimeInfo(&currentHour, &currentMinute, &currentSecond); // Read hardware RTC registers
    return ((u32)currentHour * SECONDS_PER_HOUR) + ((u32)currentMinute * SECONDS_PER_MINUTE) + (u32)currentSecond; // Convert to seconds
}

// Safely increment seconds with day wrap handling
u32 AddSeconds(u32 baseSeconds, u32 secondsToAdd)
{
    baseSeconds += secondsToAdd;            // Add offset duration
    if(baseSeconds >= SECONDS_PER_DAY) baseSeconds %= SECONDS_PER_DAY; // Wrap around 24-hour midnight boundary
    return baseSeconds;                     // Return normalized time
}

// Calculate delta seconds across potential rollover
u32 GetElapsedSeconds(u32 currentSeconds, u32 startSeconds)
{
    if(currentSeconds >= startSeconds) return currentSeconds - startSeconds; // Return direct difference
    return (SECONDS_PER_DAY - startSeconds) + currentSeconds; // Compute difference across midnight
}

// Update LCD screen contents without visual flicker
void DisplayNormalScreen(void)
{
    s32 currentHour, currentMinute, currentSecond, currentDate, currentMonth, currentYear;
    u32 hydrationPercentage, currentSeconds, elapsedSecondsSinceBase, displayAlternationMode;
    u32 scheduledReminderHour, scheduledReminderMinute;

    GetRTCTimeInfo(&currentHour, &currentMinute, &currentSecond); // Get current time
    GetRTCDateInfo(&currentDate, &currentMonth, &currentYear);   // Get current date

    hydrationPercentage = (targetGlassCount != 0) ? ((consumedGlasses * 100) / targetGlassCount) : 0; // Calculate hydration percentage
    currentSeconds = GetCurrentSeconds();   // Read current second count

    CmdLCD(GOTO_LINE1_POS0);                // Set cursor to line 1 start
    charLCD((currentHour / 10) + '0');      // Line 1: Write HH tens
    charLCD((currentHour % 10) + '0');      // Line 1: Write HH units
    charLCD(':');                           // Write separator
    charLCD((currentMinute / 10) + '0');    // Line 1: Write MM tens
    charLCD((currentMinute % 10) + '0');    // Line 1: Write MM units
    charLCD(':');                           // Write separator
    charLCD((currentSecond / 10) + '0');    // Line 1: Write SS tens
    charLCD((currentSecond % 10) + '0');    // Line 1: Write SS units
    charLCD(' ');                           // Space separator
    charLCD((currentDate / 10) + '0');      // Line 1: Write DD tens
    charLCD((currentDate % 10) + '0');      // Line 1: Write DD units
    charLCD('/');                           // Write date separator
    charLCD((currentMonth / 10) + '0');     // Line 1: Write MM tens
    charLCD((currentMonth % 10) + '0');     // Line 1: Write MM units
    charLCD(' ');                           // Pad remaining width
    charLCD(' ');                           // Keep exactly 16 characters

    CmdLCD(GOTO_LINE2_POS0);                // Move cursor to line 2

    if(isReminderAlertActive)               // Check if active alert needs display
    {
        strLCD("  DRINK WATER   ");          // Render centered reminder notification
        return;                             // Skip regular stats display
    }

    if(consumedGlasses == targetGlassCount) // Check if goal was met
    {
        strLCD("  GOAL REACHED  ");          // Render goal complete message
        return;                             // Skip regular stats display
    }

    elapsedSecondsSinceBase = GetElapsedSeconds(currentSeconds, displayCycleBaseTimestamp); // Calculate cycle reference time
    displayAlternationMode = (elapsedSecondsSinceBase / 10) % 2; // Determine 10-second alternating mode

    if(displayAlternationMode == 0)         // Mode 0: Goal, Drunk, and Hydration
    {
        charLCD('G');                       // Write goal label
        charLCD(':');                       // Write colon
        U32LCD(targetGlassCount);           // Write goal count
        charLCD(' ');                       // Add spacing
        charLCD('D');                       // Write intake label
        charLCD(':');                       // Write colon
        U32LCD(consumedGlasses);            // Write intake count
        charLCD(' ');                       // Add spacing
        charLCD('H');                       // Write hydration label
        charLCD(':');                       // Write colon
        U32LCD(hydrationPercentage);        // Write percentage value
        charLCD('%');                       // Write percentage symbol
        charLCD(' ');                       // Pad trailing spaces
        charLCD(' ');                       // Pad trailing spaces
    }
    else                                    // Mode 1: Next schedule and missed count
    {
        scheduledReminderHour = nextReminderTimestamp / SECONDS_PER_HOUR; // Calculate scheduled hour
        scheduledReminderMinute = (nextReminderTimestamp % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE; // Calculate scheduled minute

        charLCD('N');                       // Write next reminder label
        charLCD(':');                       // Write colon
        charLCD((scheduledReminderHour / 10) + '0'); // Write scheduled hour tens
        charLCD((scheduledReminderHour % 10) + '0'); // Write scheduled hour units
        charLCD(':');                       // Write time separator
        charLCD((scheduledReminderMinute / 10) + '0'); // Write scheduled minute tens
        charLCD((scheduledReminderMinute % 10) + '0'); // Write scheduled minute units
        charLCD(' ');                       // Add spacing
        charLCD('M');                       // Write missed counter label
        charLCD(':');                       // Write colon
        U32LCD(consecutiveMissedReminders); // Write missed count
        charLCD(' ');                       // Pad trailing characters
        charLCD(' ');                       // Pad trailing characters
        charLCD(' ');                       // Pad trailing characters
        charLCD(' ');                       // Pad trailing characters
    }
}

// Check push button to register glass intake
void CheckDrinkSwitch(void)
{
    if((IOPIN0 & (1 << DRINK_SW)) == 0)     // Check if drink button is grounded
    {
        delay_ms(30);                       // Debounce switch contact bounce

        if((IOPIN0 & (1 << DRINK_SW)) == 0) // Validate sustained low state
        {
            if(isReminderAlertActive)       // Register intake only during an active alert
            {
                consumedGlasses++;          // Increment consumed glass counter
                isReminderAlertActive = 0;  // Dismiss active reminder

                IOCLR1 = (1 << YELLOW_LED) | (1 << BUZZER); // Turn off warning alerts

                if(consumedGlasses >= targetGlassCount) // Evaluate if target reached
                {
                    consumedGlasses = targetGlassCount; // Cap counter at target
                    isDailyGoalAchieved = 1; // Mark goal as achieved
                    consecutiveMissedReminders = 0; // Reset consecutive misses
                    isCriticalAlarmTriggered = 0; // Clear red alarm
                    ShowGlassAnimation();   // Trigger celebration display
                }
                else                        // Progress updated but target remains
                {
                    consecutiveMissedReminders = 0; // Reset consecutive misses
                    nextReminderTimestamp = AddSeconds(nextReminderTimestamp, reminderIntervalMinutes * SECONDS_PER_MINUTE); // Schedule next
                    ShowGlassAnimation();   // Display animated glass update
                }
            }

            while((IOPIN0 & (1 << DRINK_SW)) == 0); // Wait until button is released
            delay_ms(30);                   // Debounce release event
        }
    }
}

// Compute upcoming reminder interval timestamp
void ScheduleNextReminder(void)
{
    nextReminderTimestamp = AddSeconds(GetCurrentSeconds(), reminderIntervalMinutes * SECONDS_PER_MINUTE); // Set target
}

// Activate audible and visual drinking alert
void StartReminder(void)
{
    if(consumedGlasses == targetGlassCount) // Halt operation if target already completed
    {
        isDailyGoalAchieved = 1;            // Keep status marked as complete
        isReminderAlertActive = 0;          // Keep reminders disabled
        return;                             // Return without triggering
    }

    isReminderAlertActive = 1;              // Set alert active flag
    reminderStartTimestamp = GetCurrentSeconds(); // Capture start timestamp
}

// Dismiss active alert and silence hardware
void StopReminder(void)
{
    isReminderAlertActive = 0;              // Reset alert flag
    IOCLR1 = (1 << YELLOW_LED) | (1 << BUZZER); // Turn off buzzer and yellow LED
}

// Monitor timers to trigger or timeout alerts
void CheckReminder(void)
{
    u32 currentSeconds, elapsedAlertDuration;

    if(consumedGlasses == targetGlassCount) // Abort checks if goal reached
    {
        isReminderAlertActive = 0;          // Ensure reminder is idle
        return;                             // Exit check routine
    }

    currentSeconds = GetCurrentSeconds();   // Fetch current timestamp

    if(isReminderAlertActive == 0)          // Wait for next scheduled target
    {
        if(GetElapsedSeconds(currentSeconds, nextReminderTimestamp) < (SECONDS_PER_DAY / 2) && currentSeconds >= nextReminderTimestamp) // Threshold hit
        {
            StartReminder();                // Trigger active alert
        }
        return;                             // Complete check cycle
    }

    elapsedAlertDuration = GetElapsedSeconds(currentSeconds, reminderStartTimestamp); // Measure active alert duration

    if(elapsedAlertDuration >= REMINDER_DURATION_SEC) // Check if timeout elapsed without intake
    {
        isReminderAlertActive = 0;          // Stop current alert
        IOCLR1 = (1 << YELLOW_LED) | (1 << BUZZER); // Turn off visual and audio outputs
        CompleteNeglect();                  // Register missed drink
        nextReminderTimestamp = AddSeconds(nextReminderTimestamp, reminderIntervalMinutes * SECONDS_PER_MINUTE); // Set next reminder
    }
}

// Update missed counters and check alarm threshold
void CompleteNeglect(void)
{
    consecutiveMissedReminders++;           // Increment consecutive neglect count

    if(consecutiveMissedReminders >= MAX_NEGLECTS) // Check if neglect limit exceeded
    {
        isCriticalAlarmTriggered = 1;       // Trigger critical red LED state
    }
}

// Drive LED pins based on system operational state
void UpdateLEDs(void)
{
    if(isReminderAlertActive && (consumedGlasses != targetGlassCount)) IOSET1 = (1 << YELLOW_LED); // Turn on yellow LED during reminder
    else IOCLR1 = (1 << YELLOW_LED);                                                               // Turn off yellow LED otherwise

    if(isCriticalAlarmTriggered) IOSET1 = (1 << RED_LED);                                         // Light red LED on persistent neglect
    else IOCLR1 = (1 << RED_LED);                                                                 // Extinguish red LED

    if(consumedGlasses == targetGlassCount) IOSET1 = (1 << GREEN_LED);                            // Light green LED when target completed
    else IOCLR1 = (1 << GREEN_LED);                                                               // Keep green LED off
}

// Drive buzzer pin based on reminder activity
void UpdateBuzzer(void)
{
    if(isReminderAlertActive && (consumedGlasses != targetGlassCount)) IOSET1 = (1 << BUZZER);     // Sound buzzer during reminder
    else IOCLR1 = (1 << BUZZER);                                                                   // Silence buzzer
}

// Draw a horizontal bar of filled and empty glass characters
void DisplayGlassWindow(u32 startWindowIndex)
{
    u32 cellIndex, currentGlassIndex;

    CmdLCD(GOTO_LINE2_POS0);                // Position cursor at line 2

    for(cellIndex = 0; cellIndex < 16; cellIndex++) // Iterate across 16 LCD character cells
    {
        currentGlassIndex = startWindowIndex + cellIndex; // Calculate active glass index

        if(currentGlassIndex < targetGlassCount) // Glass falls within configured goal
        {
            if(currentGlassIndex < consumedGlasses) charLCD(1); // Output filled glass glyph
            else charLCD(0);                // Output empty glass glyph
        }
        else
        {
            charLCD(' ');                   // Pad unused cells with spaces
        }
    }
}

// Render dynamic glass intake graphic animation
void ShowGlassAnimation(void)
{
    u32 currentShiftIndex, maximumShiftLimit, animationStep;

    IOCLR1 = (1 << YELLOW_LED) | (1 << BUZZER); // Suppress alerts during animation

    if(targetGlassCount <= 16)              // Goal fits inside single static LCD line
    {
        CmdLCD(CLEAR_LCD);                  // Clear display
        CmdLCD(GOTO_LINE1_POS0);            // Move cursor to line 1
        strLCD(" WATER STATUS   ");          // Print status banner
        DisplayGlassWindow(0);              // Render glasses starting at index 0
        delay_ms(1500);                     // Hold status view on screen
        IOCLR1 = (1 << YELLOW_LED) | (1 << BUZZER); // Maintain alert shutdown
        return;                             // Exit routine
    }

    maximumShiftLimit = targetGlassCount - 16; // Calculate maximum shift window

    for(animationStep = 0; animationStep <= maximumShiftLimit; animationStep++) // Scroll across total goal capacity
    {
        currentShiftIndex = animationStep;  // Shift start index
        IOCLR1 = (1 << YELLOW_LED) | (1 << BUZZER); // Maintain alert suppression
        CmdLCD(CLEAR_LCD);                  // Clear display
        CmdLCD(GOTO_LINE1_POS0);            // Move cursor to line 1
        strLCD(" WATER STATUS   ");          // Print status banner
        DisplayGlassWindow(currentShiftIndex); // Draw shifted view
        delay_ms(350);                      // Control animation frame rate
    }

    delay_ms(700);                          // Hold terminal view on display
    IOCLR1 = (1 << YELLOW_LED) | (1 << BUZZER); // Ensure alarms remain disabled
}

// Enter interactive configuration menu
void ConfigurationMode(void)
{
    GetRTCTimeInfo(&configBufferHour, &configBufferMinute, &configBufferSecond); // Load active time into temporary buffers
    GetRTCDateInfo(&configBufferDate, &configBufferMonth, &configBufferYear); // Load active date into temporary buffers

    configBufferInterval = reminderIntervalMinutes; // Cache active reminder interval
    configBufferGoal = targetGlassCount;    // Cache active goal target

    StopReminder();                         // Deactivate alerts during configuration
    ConfigurationMenu();                    // Run configuration interactive loop
}

// Main configuration interface router
void ConfigurationMenu(void)
{
    u32 keypadPressedKey, isMenuExitConfirmed = 0;

    while(isMenuExitConfirmed == 0)         // Stay in menu until explicit exit
    {
        CmdLCD(CLEAR_LCD);                  // Clear display
        CmdLCD(GOTO_LINE1_POS0);            // Position at line 1
        strLCD("1.RTC 2.GOAL");             // Print menu choices 1 and 2
        CmdLCD(GOTO_LINE2_POS0);            // Position at line 2
        strLCD("3.REM 4.EXIT");             // Print menu choices 3 and 4

        keypadPressedKey = keyscan();       // Wait for keypad input

        if(keypadPressedKey == '1') RTCConfiguration(); // Enter RTC configuration menu
        else if(keypadPressedKey == '2') GoalConfiguration(); // Enter goal modification menu
        else if(keypadPressedKey == '3') ReminderConfiguration(); // Enter reminder interval menu
        else if(keypadPressedKey == '4')    // Enter exit sequence
        {
            DisplayUpdateConfirmation();    // Prompt confirmation dialog
            keypadPressedKey = keyscan();   // Read user confirmation choice

            if(keypadPressedKey == KEY_ENTER) // Save changes
            {
                ApplyTemporarySettings();   // Commit temporary settings to system
                DisplayMessage("    UPDATED     "); // Display updated notification
                delay_ms(700);              // Display message delay
                isMenuExitConfirmed = 1;    // Break out of configuration menu
            }
            else if(keypadPressedKey == KEY_CLEAR) // Revert changes
            {
                DisplayMessage("   CANCELLED    "); // Display cancellation notification
                delay_ms(700);              // Display message delay
                isMenuExitConfirmed = 1;    // Break out of configuration menu
            }
        }
    }
}

// Submenu for RTC time and date settings
void RTCConfiguration(void)
{
    u32 keypadPressedKey;

    while(1)
    {
        CmdLCD(CLEAR_LCD);                  // Clear display
        CmdLCD(GOTO_LINE1_POS0);            // Position at line 1
        strLCD("1.TIME 2.DATE");            // Print options
        CmdLCD(GOTO_LINE2_POS0);            // Position at line 2
        strLCD("     *BACK");               // Print back action

        keypadPressedKey = keyscan();       // Wait for keypad entry

        if(keypadPressedKey == '1') TimeConfiguration(); // Launch time configuration
        else if(keypadPressedKey == '2') DateConfiguration(); // Launch date configuration
        else if(keypadPressedKey == KEY_CLEAR) return; // Return to previous menu
    }
}

// Modify hour, minute, and second parameters
void TimeConfiguration(void)
{
    u32 keypadPressedKey;

    while(1)
    {
        CmdLCD(CLEAR_LCD);                  // Clear display
        CmdLCD(GOTO_LINE1_POS0);            // Position at line 1
        strLCD("TIME");                     // Title banner
        CmdLCD(GOTO_LINE2_POS0);            // Position at line 2
        strLCD("1.HH 2.MM 3.SS");           // Print time choices

        keypadPressedKey = keyscan();       // Read selection

        if(keypadPressedKey == '1')      configBufferHour   = (s32)ReadConfigValue("ENTER HH(0-23)", (u32)configBufferHour, MIN_HOUR, MAX_HOUR); // Set hours
        else if(keypadPressedKey == '2') configBufferMinute = (s32)ReadConfigValue("ENTER MM(0-59)", (u32)configBufferMinute, MIN_MINUTE, MAX_MINUTE); // Set minutes
        else if(keypadPressedKey == '3') configBufferSecond = (s32)ReadConfigValue("ENTER SS(0-59)", (u32)configBufferSecond, MIN_SECOND, MAX_SECOND); // Set seconds
        else if(keypadPressedKey == KEY_CLEAR) return; // Return to prior menu
    }
}

// Modify day and month parameters
void DateConfiguration(void)
{
    u32 keypadPressedKey;

    while(1)
    {
        CmdLCD(CLEAR_LCD);                  // Clear display
        CmdLCD(GOTO_LINE1_POS0);            // Position at line 1
        strLCD("DATE");                     // Title banner
        CmdLCD(GOTO_LINE2_POS0);            // Position at line 2
        strLCD("1.DATE 2.MONTH");           // Print date choices

        keypadPressedKey = keyscan();       // Read selection

        if(keypadPressedKey == '1')      configBufferDate  = (s32)ReadConfigValue("ENTER DATE(1-31)", (u32)configBufferDate, MIN_DATE, MAX_DATE); // Set date
        else if(keypadPressedKey == '2') configBufferMonth = (s32)ReadConfigValue("ENTER MON(1-12)", (u32)configBufferMonth, MIN_MONTH, MAX_MONTH); // Set month
        else if(keypadPressedKey == KEY_CLEAR) return; // Return to prior menu
    }
}

// Set reminder duration interval
void ReminderConfiguration(void)
{
    configBufferInterval = ReadConfigValue("ENTER MIN", configBufferInterval, MIN_REMINDER, MAX_REMINDER); // Read interval
}

// Set daily target water goal
void GoalConfiguration(void)
{
    configBufferGoal = ReadConfigValue("ENTER GOAL", configBufferGoal, MIN_GOAL, MAX_GOAL); // Read goal value
}

// Read numeric input from keypad with validation
u32 ReadConfigValue(s8 *promptTitle, u32 fallbackValue, u32 allowedMin, u32 allowedMax)
{
    u32 parsedInputNumber = 0, enteredDigitCount = 0, keypadPressedKey;

    CmdLCD(CLEAR_LCD);                      // Clear LCD
    CmdLCD(GOTO_LINE1_POS0);                // Set line 1
    strLCD(promptTitle);                    // Show field title
    CmdLCD(GOTO_LINE2_POS0);                // Set line 2

    while(1)
    {
        keypadPressedKey = keyscan();       // Read keypad key

        if(keypadPressedKey == KEY_SKIP) return fallbackValue; // Return original value on skip

        if(keypadPressedKey == KEY_CLEAR)   // Clear entry buffer
        {
            parsedInputNumber = 0;          // Reset number
            enteredDigitCount = 0;          // Reset digit count
            CmdLCD(GOTO_LINE2_POS0);        // Reset line 2 cursor
            strLCD("                ");     // Blank entry line
            CmdLCD(GOTO_LINE2_POS0);        // Reset cursor
            continue;                       // Continue input loop
        }

        if(keypadPressedKey == KEY_BACKSPACE) // Handle character deletion
        {
            if(enteredDigitCount > 0)       // Check if characters exist to delete
            {
                parsedInputNumber /= 10;    // Pop last digit
                enteredDigitCount--;        // Decrement count
                CmdLCD(GOTO_LINE2_POS0);    // Reset line 2 cursor
                strLCD("                "); // Blank entry line
                CmdLCD(GOTO_LINE2_POS0);    // Reset cursor
                if(enteredDigitCount > 0) U32LCD(parsedInputNumber); // Print updated value
            }
            continue;                       // Continue input loop
        }

        if(keypadPressedKey == KEY_ENTER)   // Process input submission
        {
            if(enteredDigitCount > 0 && parsedInputNumber >= allowedMin && parsedInputNumber <= allowedMax) return parsedInputNumber; // Return validated number

            DisplayInvalidInput();          // Show invalid input error
            parsedInputNumber = 0;          // Reset input
            enteredDigitCount = 0;          // Reset length
            CmdLCD(CLEAR_LCD);              // Clear display
            CmdLCD(GOTO_LINE1_POS0);        // Set line 1
            strLCD(promptTitle);            // Re-render title
            CmdLCD(GOTO_LINE2_POS0);        // Reset line 2
            continue;                       // Loop back for new input
        }

        if(keypadPressedKey >= '0' && keypadPressedKey <= '9') // Process numeric key presses
        {
            if(enteredDigitCount < 4)       // Restrict maximum length to 4 digits
            {
                parsedInputNumber = (parsedInputNumber * 10) + (keypadPressedKey - '0'); // Append digit
                enteredDigitCount++;        // Increment length
                CmdLCD(GOTO_LINE2_POS0);    // Reset line 2
                strLCD("                "); // Clear line
                CmdLCD(GOTO_LINE2_POS0);    // Reset cursor
                U32LCD(parsedInputNumber);  // Display updated integer
            }
        }
    }
}

// Commit temporary user configurations to system variables
void ApplyTemporarySettings(void)
{
    SetRTCTimeInfo(configBufferHour, configBufferMinute, configBufferSecond); // Write new time into hardware RTC
    SetRTCDateInfo(configBufferDate, configBufferMonth, configBufferYear); // Write new date into hardware RTC

    reminderIntervalMinutes = configBufferInterval; // Save reminder duration
    targetGlassCount        = configBufferGoal;     // Save daily water goal

    consumedGlasses            = 0;         // Reset current intake progress
    consecutiveMissedReminders = 0;         // Clear neglect count
    isCriticalAlarmTriggered   = 0;         // Clear alert states
    isReminderAlertActive      = 0;         // Reset reminder active flag
    isDailyGoalAchieved        = 0;         // Reset goal completion status

    IOCLR1 = (1 << YELLOW_LED) | (1 << BUZZER); // Turn off indicators

    ScheduleNextReminder();                 // Recalculate upcoming reminder
    displayCycleBaseTimestamp = GetCurrentSeconds(); // Reset display toggle baseline
}

// Show invalid entry warning message
void DisplayInvalidInput(void)
{
    CmdLCD(CLEAR_LCD);                      // Clear LCD
    CmdLCD(GOTO_LINE1_POS0);                // Set line 1
    strLCD("INVALID INPUT");                // Show error banner
    CmdLCD(GOTO_LINE2_POS0);                // Set line 2
    strLCD("TRY AGAIN");                    // Show retry instruction
    delay_ms(1000);                         // Hold message display
}

// Show update confirmation options
void DisplayUpdateConfirmation(void)
{
    CmdLCD(CLEAR_LCD);                      // Clear LCD
    CmdLCD(GOTO_LINE1_POS0);                // Set line 1
    strLCD("UPDATE SETTINGS?");             // Print question prompt
    CmdLCD(GOTO_LINE2_POS0);                // Set line 2
    strLCD("=YES *NO");                     // Print keypad options
}

// Print a standalone message on Line 1
void DisplayMessage(s8 *statusMessage)
{
    CmdLCD(CLEAR_LCD);                      // Clear LCD
    CmdLCD(GOTO_LINE1_POS0);                // Set line 1
    strLCD(statusMessage);                  // Print string message
}

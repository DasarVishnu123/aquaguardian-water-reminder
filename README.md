🎯 Features
Automatic water-drinking reminders
Configurable daily water goal
Configurable reminder interval
RTC-based time management
16x2 LCD display
Drink confirmation push button
Buzzer notification
LED status indication
Missed-reminder tracking
Daily reset at midnight
Hydration percentage calculation
Glass animation using LCD CGRAM
External interrupt based configuration mode
ARM7 LPC2148 based embedded implementation
🛠️ Hardware Used
Component	Description
LPC2148	ARM7TDMI-S microcontroller
16x2 LCD	Displays time and hydration information
RTC	Maintains current time and date
Push Button	Confirms water consumption
Configuration Switch	Enters configuration mode
Buzzer	Provides drinking reminder
Yellow LED	Indicates active reminder
Red LED	Indicates missed reminders
Green LED	Indicates daily goal reached
Keypad	Used for configuration
Proteus	Circuit simulation
🔌 Pin Configuration
Switches
Pin	Function
P0.0	Drink confirmation switch
P0.3 / EINT1	Configuration switch
Outputs
Pin	Function
P1.16	Buzzer
P1.17	Yellow LED
P1.18	Red LED
P1.19	Green LED
⚙️ Working Principle

The system works using the following sequence:

                 ┌─────────────────┐
                 │    Power ON     │
                 └────────┬────────┘
                          │
                          ▼
                 ┌─────────────────┐
                 │ Initialize      │
                 │ LPC2148, LCD,   │
                 │ RTC, Keypad     │
                 └────────┬────────┘
                          │
                          ▼
                 ┌─────────────────┐
                 │ Display Current │
                 │ Time & Status   │
                 └────────┬────────┘
                          │
                          ▼
                 ┌─────────────────┐
                 │ Reminder Time   │
                 │ Reached?        │
                 └──────┬───┬──────┘
                        │No │Yes
                        │   │
                        │   ▼
                        │ ┌─────────────────┐
                        │ │ Buzzer + Yellow │
                        │ │ LED ON          │
                        │ └────────┬────────┘
                        │          │
                        │          ▼
                        │ ┌─────────────────┐
                        │ │ Drink Switch    │
                        │ │ Pressed?        │
                        │ └──────┬───┬──────┘
                        │        │Yes│No
                        │        │   │
                        │        ▼   ▼
                        │   ┌──────┐ ┌────────────┐
                        │   │Count │ │ Missed     │
                        │   │Drink │ │ Reminder   │
                        │   └──┬───┘ └─────┬──────┘
                        │      │            │
                        └──────┴────────────┘
                               │
                               ▼
                       ┌─────────────────┐
                       │ Update Hydration│
                       │ Status          │
                       └────────┬────────┘
                                │
                                ▼
                       ┌─────────────────┐
                       │ Goal Reached?   │
                       └──────┬─────┬────┘
                              │No   │Yes
                              │     │
                              │     ▼
                              │  Green LED
                              │  ON
                              │
                              ▼
                       Next Reminder
💧 Hydration Calculation

The hydration percentage is calculated using:

Hydration % = (Drunk Glasses / Daily Goal) × 100

For example:

Daily Goal = 10 glasses
Consumed   = 6 glasses

Hydration = (6 / 10) × 100
          = 60%

The LCD displays the information in the form:

G:10 D:6 H:60%

Where:

G = Daily Goal
D = Glasses Drunk
H = Hydration Percentage
⏰ Reminder System

The reminder interval can be configured by the user.

For example:

Water Goal      : 10 glasses
Reminder        : Every 60 minutes

The RTC is used to determine when the next reminder should occur.

When the reminder time is reached:

Buzzer     → ON
Yellow LED → ON
LCD        → DRINK WATER

The user has a limited time to press the drink confirmation switch.

🚨 Missed Reminder

If the user does not press the drink switch within the reminder duration, the reminder is considered missed.

The system maintains a missed-reminder count.

Example:

NR=14:00 MC=2

Where:

NR = Next Reminder
MC = Missed Count

After consecutive missed reminders, the red LED is activated.

🟢 Daily Goal Reached

When the consumed water reaches the configured daily goal:

Drunk >= Goal

The system:

Stops future reminders
Turns OFF the yellow LED
Turns OFF the buzzer
Turns ON the green LED

Example:

G:10 D:10 H:100%
🌙 Midnight Reset

At the beginning of a new day, the daily consumption information is reset.

The following values are reset:

Drunk glasses       → 0
Missed reminders    → 0
Reminder status     → Reset
Alarm status        → Reset

The configured:

Water Goal
Reminder Interval

are retained.

⚙️ Configuration Mode

The configuration mode is entered using the EINT1 external interrupt.

P0.3
 │
 ▼
EINT1
 │
 ▼
Interrupt Service Routine
 │
 ▼
Configuration Request Flag
 │
 ▼
Main Loop
 │
 ▼
Configuration Mode

The configuration menu allows the user to modify:

1. RTC Configuration
2. Reminder Configuration
3. Water Goal Configuration
4. Exit
🕐 RTC Configuration

The user can configure:

Time
Date
Day

Example:

TIME
DATE
DAY

The RTC provides the current time used by the reminder system.

⌨️ Keypad Controls
Key	Function
=	Enter / Confirm
*	Clear / Cancel
-	Backspace
/	Skip / Keep Previous Value
🖥️ LCD Display
Normal Display
10:25:30 14/08
G:10 D:4 H:40%
Reminder Display
10:26:00 14/08
DRINK WATER
Reminder Information
10:30:00 14/08
NR=11:30 MC=1
🥤 Glass Animation

AquaGuardian uses LCD CGRAM custom characters to display water glasses.

Example:

Goal = 5
Drunk = 2

Display:

█ █ □ □ □

Where:

█ = Filled glass
□ = Empty glass

For larger goals, the glasses can be displayed using a scrolling window.

💻 Software

The project is developed using:

Embedded C
Keil µVision
ARM7 LPC2148
Proteus
LCD driver
Keypad driver
RTC driver
External interrupt
VIC interrupt controller
📁 Project Structure

A typical project structure is:

AquaGuardian/
│
├── main.c
├── config.h
├── types.h
├── MACROS.h
│
├── LCD.c
├── LCD.h
├── LCD_defines.h
│
├── RTC.c
├── RTC.h
│
├── KPM.c
├── KPM.h
│
├── delay.c
├── dealy.h
│
├── Proteus/
│   └── AquaGuardian.pdsprj
│
└── README.md
🔧 Configuration Parameters

Important parameters can be configured in config.h.

Example:

#define WATER_GOAL              10
#define INITIAL_DRUNK           0
#define REMINDER_MINUTES        60

#define REMINDER_DURATION_SEC   5
#define MAX_NEGLECTS            3

#define START_HOUR              10
#define START_MIN               0
#define START_SEC               0

These values make the project easier to modify and test.

🧪 Simulation

The project can be simulated using Proteus.

During simulation:

RTC provides the current time.
LCD displays the system status.
When the reminder interval expires, the buzzer and yellow LED activate.
Pressing the drink switch records water consumption.
Missing the reminder increases the missed count.
Reaching the daily goal activates the green LED.
Configuration can be entered using the EINT1 switch.
🚀 Future Improvements

Possible future improvements include:

EEPROM/Flash storage for configuration
RTC backup battery support
UART debugging interface
CAN-based monitoring
Mobile application connectivity
IoT/cloud water-consumption tracking
Low-power sleep mode
Improved graphical LCD interface
Automatic date/day validation
👨‍💻 Project Type

Embedded Systems / ARM7 Microcontroller Project

Technologies
Embedded C
ARM7
LPC2148
RTC
LCD
Keypad
External Interrupt
Proteus
Keil

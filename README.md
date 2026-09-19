# AquaGuardian - Water Reminder System

![LPC2148](https://img.shields.io/badge/Microcontroller-LPC2148-blue)
![ARM7](https://img.shields.io/badge/Architecture-ARM7-orange)
![Embedded C](https://img.shields.io/badge/Language-Embedded%20C-green)
![Keil](https://img.shields.io/badge/IDE-Keil%20µVision-purple)
![Proteus](https://img.shields.io/badge/Simulation-Proteus-red)
![Embedded Systems](https://img.shields.io/badge/Domain-Embedded%20Systems-lightgrey)

## Overview

AquaGuardian is an embedded water reminder system developed using the **LPC2148 ARM7 microcontroller**.

The system reminds the user to drink water at configurable time intervals and keeps track of daily water consumption.

It uses an RTC for time management, a 16x2 LCD for displaying information, a keypad for configuration, a push button for confirming water intake, and LEDs and a buzzer for notifications.

The system also provides a configuration mode using the **EINT1 external interrupt**.

---

## Features

- Automatic water-drinking reminders
- Configurable daily water goal
- Configurable reminder interval
- RTC-based time and date management
- Water consumption tracking
- Hydration percentage calculation
- Missed reminder detection
- Buzzer notification
- Yellow LED for active reminder
- Red LED for missed reminders
- Green LED when daily goal is reached
- 16x2 LCD display
- Keypad-based configuration
- External interrupt for entering configuration mode
- Daily reset functionality
- LCD CGRAM based glass display
- Proteus simulation support

---

## Hardware

| Component | Description |
|---|---|
| Microcontroller | LPC2148 ARM7 |
| LCD | 16x2 Character LCD |
| Keypad | Matrix Keypad |
| RTC | LPC2148 Internal RTC |
| Buzzer | Audio reminder |
| Yellow LED | Active reminder indication |
| Red LED | Missed reminder indication |
| Green LED | Daily goal indication |
| Push Button | Water intake confirmation |
| Configuration Switch | Enters configuration mode |

---

## Pin Configuration

### Input Pins

| Peripheral | LPC2148 Pin | Description |
|---|---|---|
| Drink Button | P0.0 | Water intake confirmation |
| Configuration Switch | P0.3 / EINT1 | Enter configuration mode |

### Output Pins

| Peripheral | LPC2148 Pin | Description |
|---|---|---|
| Buzzer | P1.16 | Reminder alert |
| Yellow LED | P1.17 | Active reminder |
| Red LED | P1.18 | Missed reminder |
| Green LED | P1.19 | Daily goal reached |

---

## Block Diagram

```text
                         +----------------------+
                         |      LPC2148         |
                         |       ARM7           |
                         |   Main Controller    |
                         +----------+-----------+
                                    |
             +----------------------+----------------------+
             |                      |                      |
             |                      |                      |
             v                      v                      v
      +-------------+       +-------------+        +-------------+
      |     RTC     |       |    16x2     |        |   Keypad    |
      | Time / Date |       |     LCD     |        | Configuration|
      +-------------+       +-------------+        +-------------+
             |
             |
             v
      +---------------------+
      |  Reminder Controller |
      +----------+----------+
                 |
        +--------+--------+
        |        |        |
        v        v        v
   +---------+ +------+ +-------------+
   | Buzzer  | | LEDs | | Drink Button|
   +---------+ +------+ +-------------+

Working Principle

The system continuously monitors the RTC time.

When the configured reminder time is reached:

The yellow LED is turned ON.
The buzzer is turned ON.
The LCD displays DRINK WATER.
The user presses the drink confirmation button.
The consumed-glass count is incremented.
The hydration percentage is updated.
The next reminder is scheduled.

If the user does not press the drink button within the configured reminder duration, the reminder is treated as missed.

The missed-reminder count is incremented and the red LED is activated according to the configured alarm condition.

When the daily water goal is reached:

The buzzer is turned OFF.
The yellow LED is turned OFF.
The green LED is turned ON.
Further reminders are stopped for the current day.
LCD Display
Normal Display
10:25:30 14/08
G:10 D:4 H:40%

Where:

G = Daily water goal
D = Number of glasses consumed
H = Hydration percentage
Reminder Display
10:26:00 14/08
DRINK WATER
Next Reminder and Missed Count
10:30:00 14/08
NR=11:30 MC=1

Where:

NR = Next Reminder
MC = Missed Count
Hydration Calculation

The hydration percentage is calculated using:

Hydration % = (Consumed Glasses / Daily Goal) × 100

For example:

Daily Goal = 10
Consumed   = 4

Hydration = (4 / 10) × 100
          = 40%

LCD display:

G:10 D:4 H:40%
Water Glass Display

The LCD uses CGRAM custom characters to display water glasses.

For example:

Goal  = 5
Drunk = 2

The display represents:

[Filled] [Filled] [Empty] [Empty] [Empty]

For larger water goals, the glasses can be displayed using a scrolling window.

Reminder System

The reminder interval can be configured by the user.

For example:

Water Goal       : 10 glasses
Reminder Interval: 60 minutes

When the reminder time is reached:

Buzzer      -> ON
Yellow LED  -> ON
LCD         -> DRINK WATER

The user can then press the drink button to confirm water consumption.

Missed Reminder

If the user does not confirm water consumption within the configured reminder duration:

Reminder
   |
   v
No confirmation
   |
   v
Reminder timeout
   |
   v
Missed count +1
   |
   v
Red LED / Alarm

The missed-reminder count can be displayed on the LCD.

Example:

NR=11:30 MC=2
Daily Goal

When the consumed water reaches the configured daily goal:

Consumed Glasses >= Daily Goal

The system:

Buzzer      -> OFF
Yellow LED  -> OFF
Green LED   -> ON
Reminders   -> STOP

Example:

G:10 D:10 H:100%

The system waits for the next daily reset before starting a new day's consumption tracking.

Midnight Reset

At the beginning of a new day, daily consumption-related information is reset.

The following values are reset:

Consumed glasses
Missed reminder count
Reminder status
Alarm status

The configured values are retained:

Water goal
Reminder interval
Configuration Mode

The configuration mode is entered using the EINT1 external interrupt.

The configuration switch is connected to:

P0.3 -> EINT1

The process is:

Configuration Switch
          |
          v
       EINT1
          |
          v
Interrupt Service Routine
          |
          v
Configuration Request Flag
          |
          v
       Main Loop
          |
          v
Configuration Mode

The interrupt service routine only sets a configuration request flag.

The actual keypad and LCD operations are performed from the main program.

Configuration Menu

The configuration menu provides the following options:

1. RTC Configuration
2. Reminder Configuration
3. Water Goal Configuration
4. Exit
RTC Configuration

The user can configure:

Time
Date
Day
Reminder Configuration

The user can configure the reminder interval.

Water Goal Configuration

The user can configure the daily water-consumption goal.

Keypad Controls
Key	Function
=	Enter / Confirm
*	Clear / Cancel
-	Backspace
/	Skip / Keep Previous Value
Software

The project is developed using:

Embedded C
ARM7 LPC2148
Keil µVision
Proteus
LPC2148 RTC
LCD Driver
Keypad Driver
External Interrupt
VIC Interrupt Controller
Software Modules
Module	Purpose
main.c	Main application logic
config.h	Configuration constants and function prototypes
LCD.c	LCD interface
RTC.c	RTC interface
KPM.c	Keypad interface
delay.c	Delay functions
MACROS.h	Common macros
types.h	User-defined data types
Project Structure
AquaGuardian/
|
|-- README.md
|
|-- main.c
|-- config.h
|-- types.h
|-- MACROS.h
|
|-- LCD.c
|-- LCD.h
|-- LCD_defines.h
|
|-- RTC.c
|-- RTC.h
|
|-- KPM.c
|-- KPM.h
|
|-- delay.c
|-- delay.h
|
|-- Proteus/
|   `-- AquaGuardian.pdsprj
|
`-- Documentation/
    |-- circuit_diagram.png
    |-- block_diagram.png
    |
    `-- screenshots/
        |-- normal_display.png
        |-- reminder.png
        |-- configuration.png
        `-- glass_display.png
How to Run
1. Open the Project

Open the LPC2148 project in Keil µVision.

2. Configure the Parameters

Open:

config.h

Important parameters include:

#define WATER_GOAL              10
#define INITIAL_DRUNK           0
#define REMINDER_MINUTES        60
#define REMINDER_DURATION_SEC   5
#define MAX_NEGLECTS            3

For quick Proteus testing, the reminder interval can be temporarily reduced.

Example:

#define REMINDER_MINUTES 2

After testing, it can be changed back to:

#define REMINDER_MINUTES 60
3. Build the Project

In Keil µVision:

Project -> Build Target

or press:

F7

Make sure the project builds without errors.

4. Generate the HEX File

Enable HEX file generation in the Keil project settings.

The generated HEX file will be used in Proteus.

5. Open Proteus

Open the Proteus project:

Proteus/AquaGuardian.pdsprj

Load the generated HEX file into the LPC2148 microcontroller.

6. Run the Simulation

Start the Proteus simulation.

The system will:

Initialize the LPC2148.
Initialize the LCD.
Initialize the RTC.
Initialize the keypad.
Initialize the EINT1 interrupt.
Display the system information.
Monitor the reminder schedule.
Detect water consumption.
Update hydration information.
Reset daily status at midnight.

Testing
Test 1 - Normal Operation
Power ON
   |
   v
System Initialization
   |
   v
RTC Starts
   |
   v
LCD Displays Time
   |
   v
Hydration Status

Test 2 - Reminder
Reminder Time Reached
          |
          v
     Buzzer ON
          |
          v
    Yellow LED ON
          |
          v
     DRINK WATER

Test 3 - Drink Confirmation
Press Drink Button
        |
        v
Consumed Count +1
        |
        v
Hydration Updated
        |
        v
Reminder OFF

Test 4 - Missed Reminder
No Drink Confirmation
        |
        v
Reminder Timeout
        |
        v
Missed Count +1
        |
        v
Red LED / Alarm Logic

Test 5 - Goal Reached
Consumed Count >= Goal
          |
          v
      Green LED ON
          |
          v
       Buzzer OFF
          |
          v
    Reminders STOP

Test 6 - Configuration
Press Configuration Switch
            |
            v
          EINT1
            |
            v
    Configuration Menu
            |
            v
      Modify Settings
            |
            v
          Apply

Test 7 - Midnight Reset
23:59:59
    |
    v
New Day
    |
    v
Daily Values Reset
Circuit Diagram

The Proteus circuit diagram can be added here.

Place the image at:

Documentation/circuit_diagram.png

Then it will appear below:

Screenshots
Normal Display

Place the screenshot at:

Documentation/screenshots/normal_display.png

Water Reminder

Place the screenshot at:

Documentation/screenshots/reminder.png

Configuration Mode

Place the screenshot at:

Documentation/screenshots/configuration.png

Glass Display

Place the screenshot at:

Documentation/screenshots/glass_display.png

Embedded Concepts Demonstrated

This project demonstrates practical implementation of:

Embedded C programming
ARM7 LPC2148 programming
GPIO configuration
RTC programming
External interrupts
VIC interrupt controller
LCD interfacing
LCD CGRAM
Keypad interfacing
Switch debouncing
Bit manipulation
Time-based scheduling
State-based control
Modular programming
Proteus simulation
Future Improvements

Possible future improvements include:

EEPROM/Flash based configuration storage
RTC backup battery support
UART debugging
IoT connectivity
Mobile application
Cloud-based water-consumption monitoring
Low-power operation
Historical water-consumption logging
Improved graphical user interface
Author

Dasari Vishnu Vardhan Reddy

Embedded Systems / Firmware Enthusiast

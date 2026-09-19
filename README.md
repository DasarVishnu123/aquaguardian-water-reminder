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

# AquaGuardian - Water Reminder System

![LPC2148](https://img.shields.io/badge/Microcontroller-LPC2148-blue)
![ARM7](https://img.shields.io/badge/Architecture-ARM7-orange)
![Embedded C](https://img.shields.io/badge/Language-Embedded%20C-green)
![Keil](https://img.shields.io/badge/IDE-Keil%20µVision-purple)
![Proteus](https://img.shields.io/badge/Simulation-Proteus-red)
![Embedded Systems](https://img.shields.io/badge/Domain-Embedded%20Systems-lightgrey)

## Overview :

AquaGuardian is a smart hydration reminder system designed to help users maintain their daily water intake. The system tracks the amount of water consumed, displays hydration progress on a 16×2 LCD, provides reminders using a buzzer and LEDs, and allows the user to configure hydration goals and reminder settings using a 4×4 keypad.

## Features :

* Daily water intake tracking
* Configurable daily hydration goal
* Automatic hydration reminders
* RTC-based time and date management
* Reminder interval configuration
* Missed reminder tracking
* Buzzer notification
* Red, Yellow and Green LED status indication
* 16×2 LCD user interface
* Custom LCD characters for hydration progress
* 4×4 keypad-based configuration menu
* External interrupt for entering configuration mode
* Daily reset functionality

## Hardware

* Microcontroller: LPC2148 (ARM7)
* LCD: 16×2 character LCD
* Keypad: 4×4 matrix keypad
* RTC: LPC2148 internal RTC
* Buzzer
* Red, Yellow and Green LEDs
* Push button for water intake detection

## Pin Configuration

| Peripheral | LPC2148 Pin |
| :--- | :--- |
| Yellow LED | P1.24 |
| Green LED | P1.25 |
| Red LED | P1.26 |
| Buzzer | P1.27 |
| Drink Button | P0.23 |
| EINT0 | P0.1 |
| Keypad Rows | P1.16 – P1.19 |
| Keypad Columns | P1.20 – P1.23 |
| LCD | P0.8 – P0.18 |


#Software Modules :

The project follows a modular embedded-C design, with separate drivers for each peripheral.

Plaintext

              main.c
              │
              ├── hydration.c/h
              │      └── Hydration and reminder management
              │
              ├── rtc.c/h
              │      └── Real-Time Clock
              │
              ├── lcd.c/h
              │      └── 16×2 LCD interface
              │
              ├── kpm.c/h
              │      └── 4×4 keypad interface
              │
              ├── led.c/h
              │      └── LED control
              │
              ├── buzzer.c/h
              │      └── Buzzer control
              │
              ├── interrupt.c/h
              │      └── External interrupt handling
              │
              └── delay.c/h
                     └── Software delay functions
       
#System Operation

The system initializes the LPC2148 peripherals.

The RTC provides the current time and date.

The LCD displays hydration-related information.

The user can configure hydration settings using the keypad.

When the configured reminder interval is reached, the system activates the buzzer and LED indicators.

Pressing the water-intake button updates the consumed-water count.

Hydration progress is continuously monitored.

The external interrupt provides access to the configuration mode.

#Configuration Menu :

The external interrupt enters configuration mode. The keypad provides the following options:

1 → Set daily hydration goal

2 → Configure RTC

3 → Configure reminder interval

C → Exit configuration mode

Project Flow :
Plaintext

              ┌──────────────────┐ 
              │   System Start   │
              └────────┬─────────┘ 
                       ↓
              Initialize Peripherals 
                       ↓ 
             ┌──────────────────────┐ 
             │  Display RTC Status  │ 
             │  & Hydration Status  │
             └──────────┬───────────┘ 
                        ↓
                 Check Reminder Time 
                        ↓ 
             ┌──────────┴───────────┐ 
             │                      │
             │                      │
        Reminder Due             No Reminder 
             │                      │ 
             ↓                      ↓
        Buzzer + LED           Continue Monitoring 
             │ 
             ↓
           Drink Button? 
           /            \ 
        Yes             No 
         ↓              ↓ 
  Increase Count     Track Missed 
      │                  Reminder 
      ↓ 
  Update Progress 
      │ 
      └──────────────→ Continue
      
#Technologies Used :

Embedded C

ARM7 / LPC2148

GPIO

RTC

External Interrupts

16×2 LCD

Matrix Keypad

Buzzer

LED indicators

Modular Embedded-C Programming

#Development Environment :

ARM7 / LPC2148 development environment

Embedded C

Keil µVision

Proteus simulation

#Project Structure :

Plaintext
AquaGuardian
│
├── main.c
│
├── hydration.c
├── hydration.h
│
├── rtc.c
├── rtc.h
│
├── lcd.c
├── lcd.h
├── lcd_defines.h
│
├── kpm.c
├── kpm.h
├── kpm_defines.h
│
├── led.c
├── led.h
│
├── buzzer.c
├── buzzer.h
│
├── interrupt.c
├── interrupt.h
│
├── delay.c
├── delay.h
│
├── defines.h
├── types.h
│
└── README.md

#Learning Outcomes :

This project provided practical experience in:

Embedded C programming

ARM7 LPC2148 GPIO programming

Peripheral driver development

Interrupt handling

RTC programming

LCD interfacing

Matrix keypad interfacing

Buzzer and LED control

Modular driver-based software design

Embedded application design

#Author :

Dasari Vishnuvardhan Reddy

Embedded Systems | Embedded C | ARM7 | LPC2148

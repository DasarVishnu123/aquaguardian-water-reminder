AquaGuardian – Smart Hydration Reminder SystemAn embedded systems project developed using the LPC2148 ARM7 microcontroller to monitor daily water intake, log consumption progress, and deliver automated audio-visual hydration alerts.OverviewAquaGuardian is an intelligent hydration assistant built to help users build consistent water-drinking habits. The system continuously tracks the total glasses of water consumed versus a user-defined daily goal.Using an internal Real-Time Clock (RTC), it triggers scheduled reminders via an acoustic buzzer and a visual Yellow LED. The system logs missed reminders, alerts users of chronic neglect using a Red LED, confirms goal completion with a Green LED, and renders custom graphic glass icons on an HD44780 16×2 LCD. System configuration (time, date, target goal, and reminder frequency) can be adjusted at runtime via an external interrupt switch and a 4×4 matrix keypad.FeaturesDaily Water Intake Tracking: Real-time counter monitoring consumed glasses against a daily hydration target.Configurable Daily Hydration Goal: User-customizable intake limit (supports double-digit targets with visual scaling).Automatic Hydration Reminders: Periodically alerts the user when it is time to drink based on RTC-driven intervals.Hardware RTC Time & Date Management: Integrated real-time tracking of hours, minutes, seconds, day, and month with day rollover support.Dual-View Alternating LCD Display: Line 2 alternates every 10 seconds between target statistics (G: Goal, D: Drunk, H: Hydration %) and scheduling details (N: Next Reminder, M: Missed Count).Custom CGRAM Character Graphics: Dynamic visual rendering of empty and filled water glass icons.Missed Reminder Tracking: Automatically detects unacknowledged reminder timeouts and increments consecutive neglect metrics.Triple LED Status Indicators:Yellow LED: Active reminder alert in progress.Red LED: Warning state triggered after reaching maximum consecutive neglects.Green LED: Achievement indicator illuminated once the target goal is met.Acoustic Buzzer Alerts: Audio alerts sounding concurrently during active hydration reminder windows.External Interrupt Configuration Mode: Dedicated push-button tied to EINT1 halts regular polling to safely open the configuration menu.Input Validation & Debouncing: Hardware and software debounced switches with bounds-checking on numeric keypad entries.Hardware SpecificationsMicrocontroller: NXP LPC2148 (ARM7TDMI-S 32-bit Core, 60 MHz maximum frequency)Display: 16×2 Character LCD (HD44780 compliant, custom CGRAM glyphs)Input Matrix: 4×4 Keypad Matrix (KPM)Timebase: LPC2148 Internal Real-Time Clock (RTC with 32.768 kHz external crystal)Auditory Unit: 5V Active Acoustic BuzzerVisual Status Unit: 3× Discrete LEDs (Red, Yellow, Green)User Buttons:Push-Button (Active-LOW) for logging water intakePush-Button (Falling-edge) for triggering external configuration interrupt (EINT1)Pin ConfigurationPeripheralLPC2148 PinSignal FunctionDirectionConfiguration / ModeDrink ButtonP0.0GPIO InputInputActive-LOW, DebouncedConfig SwitchP0.3EINT1InterruptFalling-edge sensitive via VICBuzzerP1.16GPIO OutputOutputPush-Pull, Active-HIGHYellow LEDP1.17GPIO OutputOutputReminder Active AlertRed LEDP1.18GPIO OutputOutputConsecutive Neglect WarningGreen LEDP1.19GPIO OutputOutputHydration Goal CompletedKeypad MatrixGPIO PortRows / ColsIn / OutStrobed matrix scanning16×2 LCDGPIO PortData & ControlOutputHD44780 4-bit / 8-bit bus interfaceBlock DiagramPlaintext               +--------------------------------------------------+
               |               NXP LPC2148 (ARM7TDMI-S)           |
               |                                                  |
[ Drink SW ]  ---> P0.0 (GPIO Input)       Internal Hardware RTC  |
               |                            [Time, Date Tracking] |
[ Config SW ] ---> P0.3 (EINT1 Interrupt)             │           |
               |                                      ▼           |
[ 4x4 Keypad ] <-> GPIO (Matrix Interface)    Main State Engine   |
               |                             Alert State Machine  |
               |                                      │           |
               |        +-----------------------------+           |
               |        │           │           │           │     |
               +--------│-----------│-----------│-----------│-----+
                        ▼           ▼           ▼           ▼
                      P1.16       P1.17       P1.18       P1.19
                    [Buzzer]  [Yellow LED] [Red LED]  [Green LED]
                        │           │           │           │
                        +-----------+-----+-----+-----------+
                                          │
                                          ▼
                                    16×2 Character LCD
                                (Custom CGRAM Glass Glyphs)
Software ModulesThe codebase is organized in a modular embedded-C structure separating low-level register abstractions from application logic:PlaintextAquaGuardian
│
├── main.c              # Main application loop, ISR handling, and display scheduler
├── config.h            # System thresholds, pin configurations, and prototypes
├── types.h             # Architecture-specific integer typedefs (u32, s32, s8)
│
├── RTC.c / RTC.h       # LPC2148 internal Real-Time Clock configuration & reading
├── LCD.c / LCD.h       # HD44780 LCD driver, CGRAM glass loading, and formatted printing
├── LCD_defines.h       # LCD command macros and register addresses
│
├── KPM.c / KPM.h       # 4×4 Matrix Keypad scanning and keycode conversion
├── MACROS.h            # Optimized bit-manipulation macros
└── dealy.c / dealy.h   # Hardware-calibrated delay utility functions
System OperationInitialization: On reset, the microcontroller initializes GPIO directions, configures the internal RTC, registers custom empty/filled glass bitmaps in LCD CGRAM, sets up EINT1 in the VIC, and scrolls the startup marquee.Normal Monitoring: The RTC continuously updates the clock on LCD Line 1. Line 2 alternates every 10 seconds between consumed hydration data and the upcoming reminder schedule.Alert Trigger: When the current timestamp equals or passes the scheduled reminder target, the system turns ON the Yellow LED and sounds the Buzzer, displaying "DRINK WATER" on the screen.Intake Acknowledgment: Pressing the active-low drink button during an alert shuts off the audio-visual alarms, increments the consumed count, resets consecutive misses to zero, and schedules the next reminder.Neglect Handling: If 30 seconds pass without intake, the alert times out. Consecutive neglect increments by one. Reaching 3 consecutive neglects turns ON the Red LED.Goal Completion: When total glasses equal the configured target, reminders cease, the Green LED turns ON, and "GOAL REACHED" is shown on the LCD alongside an animated glass graph.Interactive Configuration: Pressing the interrupt switch enters the setup mode where RTC time, target goals, and intervals can be modified through the 4×4 keypad.Configuration MenuTriggering the EINT1 interrupt suspends normal background tasks and launches the menu on the LCD:PlaintextLine 1: 1.RTC 2.GOAL
Line 2: 3.REM 4.EXIT
Navigation Map:Key 1: Configure RTC Parameters1 → Time Configuration (HH, MM, SS)2 → Date Configuration (DD, MM)* → Return to parent menuKey 2: Set Daily Hydration Goal (Range: 1 – 99 glasses)Key 3: Set Reminder Interval (Range: 1 – 999 minutes)Key 4: Exit MenuPress = to apply and save changesPress * to cancel and discard temporary changesKeypad Input Keys:= (Enter): Confirms and submits the typed value* (Clear): Clears input buffer or navigates back- (Backspace): Removes the last entered digit/ (Skip): Bypasses the field without making changesProject FlowchartPlaintext              ┌───────────────────────────┐
              │        System Reset       │
              └─────────────┬─────────────┘
                            │
                            ▼
              ┌───────────────────────────┐
              │   Initialize Peripherals  │
              │   (GPIO, LCD, RTC, EINT1) │
              └─────────────┬─────────────┘
                            │
                            ▼
              ┌───────────────────────────┐
              │   Display RTC Time &      │
              │   Alternating Status Data │
              └─────────────┬─────────────┘
                            │
                            ▼
              ┌───────────────────────────┐
              │    Is Reminder Due?       │
              └──────┬─────────────┬──────┘
                     │             │
                    Yes            No
                     │             │
                     ▼             ▼
        ┌──────────────────────┐  Continue Polling Loop
        │ Turn ON Yellow LED   │
        │ & Sound Buzzer       │
        └────────────┬─────────┘
                     │
                     ▼
        ┌──────────────────────────┐
        │  Drink Button Pressed?   │
        └──────┬────────────┬──────┘
               │            │
              Yes           No (30s Timeout)
               │            │
               ▼            ▼
        ┌─────────────┐  ┌──────────────────────┐
        │ Increment   │  │ Increment Neglects   │
        │ Drunk Count │  │ (Neglects >= 3 ->    │
        │ & Turn OFF  │  │  Turn ON Red LED)    │
        │ Alerts      │  └──────────┬───────────┘
        └──────┬──────┘             │
               │                    │
               ▼                    ▼
        ┌──────────────────────────────────────┐
        │ Recalculate Next Reminder Timestamp  │
        └──────────────────────────────────────┘
Technologies UsedLanguage: Embedded CTarget Architecture: ARM7TDMI-S (NXP LPC2148)Core Peripherals: GPIO, Real-Time Clock (RTC), External Interrupts (EXTINT / VIC)Display Interface: HD44780 16×2 Character LCD (4-bit / 8-bit mode, CGRAM customization)Input Device: 4×4 Matrix Keypad Scanning (KPM)Indicators: Piezo Acoustic Buzzer, Discrete LEDsDevelopment EnvironmentTarget Microcontroller: NXP LPC2148Integrated Development Environment (IDE): Keil µVision (ARM Compiler Toolchain)In-System Programmer: Flash Magic (UART-based ISP programming via COM port)Hardware Debugging / Simulation: Proteus VSM / Keil Simulator / Hardware Evaluation BoardProject StructurePlaintextAquaGuardian
│
├── main.c                  # Application state engine and scheduler
├── config.h                # System definitions, pin mappings, and parameters
├── types.h                 # Standard integer width definitions
│
├── RTC.c                   # Low-level RTC hardware interface
├── RTC.h                   # RTC register functions and prototypes
│
├── LCD.c                   # LCD peripheral control and custom glyph driver
├── LCD.h                   # LCD driver interfaces
├── LCD_defines.h           # LCD instruction commands and memory addresses
│
├── KPM.c                   # Keypad scanning driver
├── KPM.h                   # Keypad lookup tables and scan functions
│
├── MACROS.h                # Register bit setting and clearing macros
├── dealy.c                 # Software calibrated delay implementations
├── dealy.h                 # Delay header definitions
│
└── README.md               # System documentation
Learning OutcomesDeveloping this project provided hands-on experience in:Bare-metal embedded C firmware development on ARM7 microcontrollers.Direct register-level programming without third-party HAL dependencies.Configuring the Vectored Interrupt Controller (VIC) for low-latency external interrupts (EINT1).Interfacing hardware RTC blocks and managing 24-hour timekeeping algorithms.Generating custom bitmap characters using LCD CGRAM memory.Writing matrix keypad row-column scanning routines with software debouncing.Designing state-driven alert schedules and failsafe neglect monitoring systems.AuthorDasari VishnuEmbedded Systems | Embedded C | ARM7 | LPC2148

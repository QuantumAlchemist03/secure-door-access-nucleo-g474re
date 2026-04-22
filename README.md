# Secure Door Access Management System

An embedded access control system built on the **NUCLEO-G474RE** development board using **Mbed OS 6** and **C++**. Developed as part of the MOD007361 Embedded Systems module at Anglia Ruskin University.

## Overview

This project implements a secure door access management system that authenticates users via a 4×4 matrix keypad and controls a door lock (simulated by the onboard LED). The system communicates with a serial terminal GUI for remote interaction and provides audio-visual feedback through a passive buzzer.

## Features

- **PIN-based authentication** — 4-digit PIN entry via physical keypad with masked serial echo
- **3-attempt lockout** — system locks after three consecutive failed PIN entries, requiring a board reset
- **Door control** — unlock/lock commands via serial terminal (simulated with onboard LED)
- **Doorbell detection** — onboard user button (PC_13) with falling-edge detection and debounce to prevent repeated triggers
- **Audio feedback** — passive buzzer driven via PWM with distinct tones for keypress, access granted, access denied, and doorbell events
- **Serial terminal GUI** — full menu-driven interface over USART2 at 115200 baud (compatible with Tera Term, PuTTY, VS Code Serial Monitor)

## Hardware

| Component | Connection |
|-----------|------------|
| NUCLEO-G474RE | Main microcontroller |
| 4×4 Matrix Keypad | R1–R4 → A0–A3, C1–C4 → A4–A5, D11–D12 |
| Passive Buzzer | GND → GND, VCC → 3V3, I/O → D13 (PWM) |
| Onboard LED (PA_5) | Door lock actuator (built-in) |
| Onboard Button (PC_13) | Doorbell sensor (built-in) |
| USB Cable | Serial communication to PC |

## Project Structure

```
├── main.cpp                 # Firmware source code
├── mbed-os.lib              # Mbed OS library reference
├── mbed_app.json            # Mbed build configuration
├── README.md                # This file
├── docs/
│   ├── limitations.docx     # 5 limitations write-up (2 addressed, 3 future work)
│   └── flow_diagram.png     # System flow diagram
└── video/
    └── demo.mp4             # 5-minute demonstration video
```

## Building and Flashing

1. Import the project into **Keil Studio Cloud** or open in **VS Code** with the Keil Studio Pack extension
2. Set the target board to **NUCLEO-G474RE**
3. Build the project (Mbed OS 6)
4. Flash the compiled binary to the board via USB drag-and-drop or the IDE flash button

## Serial Terminal Setup

1. Install the **STMicroelectronics STLink driver** (not the generic Microsoft USB Serial driver)
2. Open a serial terminal (Tera Term, PuTTY, or VS Code Serial Monitor)
3. Connect to the STLink COM port at **115200 baud**, 8N1
4. Set line endings to **CR+LF**

## Usage

1. Power on the board — the welcome banner appears on the serial terminal
2. Enter the 4-digit PIN using the keypad (default: `2580`)
3. Upon successful authentication, use the serial terminal commands:
   - `U` — Unlock door (LED on)
   - `L` — Lock door (LED off)
   - `Q` — Logout (returns to PIN prompt)
4. Press the onboard blue button to trigger the doorbell

## Limitations

Five system limitations are documented in `docs/limitations.docx`:

**Addressed in implementation:**
1. No user authentication → PIN-based auth with keypad and lockout
2. No feedback on door state → multi-modal audio-visual feedback via buzzer and serial messages

**Future work:**
3. Single-factor authentication only (recommend RFID/TOTP as second factor)
4. No persistent access log or audit trail
5. No physical security or tamper protection on hardware

## Tools and Environment

- **IDE:** Keil Studio Cloud / VS Code with Keil Studio Pack
- **Framework:** Mbed OS 6
- **Language:** C++
- **Board:** STM32 NUCLEO-G474RE
- **Terminal:** Tera Term / VS Code Serial Monitor

## License

This project was developed for academic assessment purposes.

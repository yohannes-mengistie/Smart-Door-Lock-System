# 🔐 Smart Door Lock System

## 📘 Introduction

The **Smart Door Lock System** is a microcontroller-based security solution that enables password-protected access control using a 4x3 matrix keypad and LCD display. Built on the **LPC2138 (ARM7)** microcontroller, this system controls a **DC motor** to simulate the locking/unlocking mechanism and provides **visual (LEDs)** and **audible (buzzer)** feedback. It is designed for simulation in **Proteus** or for physical implementation with real components.

---

## 🔧 Hardware Requirements

| Component                | Description                                                                 |
|--------------------------|-----------------------------------------------------------------------------|
| **LPC2138 MCU**          | 32-bit ARM7TDMI-S, 12 MHz crystal, 3.3V                                     |
| **16x2 LCD**             | Displays prompts, status, and feedback                                      |
| **4x3 Keypad**           | For entering passwords (0-9, *, #)                                          |
| **DC Motor**             | Simulates door lock (clockwise/anti-clockwise rotation)                    |
| **L293D Motor Driver**   | Dual H-Bridge to drive motor in both directions                             |
| **LEDs**                 | Red: access denied, Green: access granted                                   |
| **Active Buzzer**        | Feedback for keypress, correct/wrong password, lockout                      |
| **Crystal Oscillator**   | 12 MHz with 2x33pF capacitors for LPC2138 clock                             |
| **Resistors**            | 10kΩ (pull-ups), 1kΩ (LEDs), 330Ω (LCD backlight)                           |
| **Capacitors**           | 100nF, 100µF, 33pF for power and clock stability                            |
| **Diodes (1N4007)**      | Flyback protection for motor                                                |
| **Power Supply**         | 3.3V (LPC2138), 5V (LCD, L293D), optional 12V (motor)                       |
| **10kΩ Potentiometer**   | For LCD contrast adjustment                                                 |

---

## 🛠️ Component Functionality

### 1. LPC2138 Microcontroller
- Reads keypad input via GPIO.
- Controls LCD, motor driver, buzzer, and LEDs.
- Implements password logic and lockout mechanism.

### 2. 16x2 LCD
- Shows system messages:
  - `"SMART DOOR LOCK"` at startup.
  - `"ENTER PASSWORD:"`, `"ACCESS GRANTED"`, `"ACCESS DENIED"`, etc.
- Driven in 8-bit mode (P0.0–P0.7, P0.10, P0.11).

### 3. 4x3 Keypad
- Inputs password.
- Uses scanning method (P1.16–P1.22).
- `#` key resets input.

### 4. DC Motor + L293D Driver
- Locks (CW) or unlocks (CCW) the door.
- Controlled via P0.8, P0.9, P0.21.
- Includes back EMF protection with diodes.

### 5. LEDs
- Red: flashes on incorrect password.
- Green: indicates access granted.
- Controlled by P0.12 (Red), P0.13 (Green).

### 6. Buzzer
- Audible feedback on:
  - Keypress (50ms beep)
  - Correct password (500ms, 1000Hz)
  - Wrong password (200ms, 500Hz)
  - Lockout (continuous + 500ms beep)

### 7. Power System
- 3.3V for LPC2138 core.
- 5V for peripherals.
- Capacitors for filtering and stability.
- Potentiometer for LCD contrast.

---

## 🧪 Simulation: Proteus Design

The project is fully simulative in **Proteus**, allowing easy testing of:
- Keypad input logic
- LCD feedback
- Motor direction control
- Password attempts and lockout

---

## ✅ Features

- Secure password-based entry.
- Resettable input via `#`.
- Feedback via LEDs and buzzer.
- Lockout after 3 incorrect attempts.
- Proteus-compatible and hardware-ready.

---


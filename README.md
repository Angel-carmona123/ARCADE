# 🕹️ MSP430 Mini Arcade Console

This project is a **mini arcade-style gaming console** developed using the **MSP430** microcontroller and written entirely in **C**. It features a simple user interface with a main menu, multiple built-in games, and button-based controls. The goal was to create an interactive and responsive embedded system with limited resources, focusing on low-level programming and real-time behavior.

---

## 🎯 Project Goals

- Build a basic arcade system using an ultra-low-power microcontroller.
- Write efficient C code that interacts directly with hardware.
- Implement multiple games.
- Use interrupts and timers to manage input and game updates.
- Demonstrate embedded systems development with limited hardware resources.

---

## 🧱 System Overview

| Component         | Description |
|------------------|-------------|
| **Main Menu**     | Displays a list of available games. Users can navigate with buttons. |
| **Game Modules**  | Each game is a standalone C module loaded from the menu. |
| **Button Inputs** | Directional movement and action buttons read via GPIO pins. |
| **Display**       | Output via LED matrix, LCD, or UART-based terminal (depending on version). |
| **Timers**        | Used for delays, animations, and debouncing. |
| **Interrupts**    | Handle real-time input and state transitions. |

---

## 🎮 Included Games

> *(Each game is modular and can be compiled independently.)*

### 1. **Reaction to enemies**
- Press the button as fast as possible when the enemies come to you.
- Measures and displays reaction time.

### 2. **Reaction to the arrow**
- User has to move the joystick in the correct direction.
- Provides feedback with the directions and the colors.

### 3. **Catch the apples**
- Apples falls from the skys and you have to move to catch them.

---

## 🖥️ Platform & Tools

- **Board**: TI MSP430 LaunchPad (e.g., MSP430G2553 or MSP430FR2433)
- **Language**: C (ANSI C89/C99)
- **Toolchain**: Code Composer Studio (CCS) or GCC for MSP430
- **Peripherals Used**:
  - GPIO (Buttons, LEDs)
  - Timers (for delays and periodic interrupts)
  - UART (optional output for debugging or terminal display)
  - LCD or LED Matrix (optional output interface)
  - Joystick inputs

---

## 🧪 Testing Strategy

- **Unit testing** of game logic using conditional compilation.
- **Button debouncing** tested through timers and edge interrupts.
- **Modular game structure** to isolate and test individual games.
- Debug output via UART (where available).

---

Sure, here is a concise README document in English based on the provided `Factory.ino` code:

---

# Dual LR1121 LoRa Module Example Project

## Overview

This project demonstrates the use of two LR1121 LoRa modules on an Arduino platform to perform dual-band LoRa communication. It supports transmission and reception modes on three frequency bands: 868MHz, 915MHz, and 2400MHz. The project includes a button to switch between different modes and a NeoPixel LED to indicate the current mode.

## Hardware Requirements

- Arduino development board (e.g., ESP32)
- Two LR1121 LoRa modules
- NeoPixel LED module
- Connecting wires

## Pin Connections
Ensure the following connections between the LR1121 modules, NeoPixel LED, and the Arduino development board:

## Pin Assignment Table
| Functional Module    | Signal Name | ESP32 Pin | Description      |
| -------------------- | ----------- | --------- | ---------------- |
| **LR1121-1**         | MISO        | 33        | SPI data input   |
|                      | MOSI        | 32        | SPI data output  |
|                      | SCK         | 25        | SPI clock        |
|                      | CS          | 27        | Chip selection   |
|                      | DIO9        | 37        | Interrupt signal |
|                      | RST         | 26        | Module reset     |
|                      | BUSY        | 36        | Status output    |
| **LR1121-2**         | MISO        | 33        | SPI data input   |
|                      | MOSI        | 32        | SPI data output  |
|                      | SCK         | 25        | SPI clock        |
|                      | CS          | 13        | Chip selection   |
|                      | DIO9        | 34        | Interrupt signal |
|                      | RST         | 21        | Module reset     |
|                      | BUSY        | 39        | Status output    |
| **Status Indicator** | LED         | 5         | SPI data input   |
| **Button**           | BOOT        | 0         |                  |

## Modes

### Mode Switching

Pressing the button switches between different modes:

- Transmit mode (868MHz)
- Transmit mode (915MHz)
- Transmit mode (2400MHz)
- Receive mode (868MHz)
- Receive mode (915MHz)
- Receive mode (2400MHz)

### Transmit Functionality

In transmit mode, the two LR1121 modules alternate sending predefined messages.

### Receive Functionality

In receive mode, both LR1121 modules attempt to receive messages, and the received data and RSSI (Received Signal Strength Indicator) are printed to the serial monitor.

### LED Status Indicators

- **Red:** Invalid configuration or transmit failure
- **Orange:** Transmit success at 868MHz
- **Yellow:** Receive success at 868MHz
- **Green:** Transmit success at 915MHz
- **Cyan:** Receive success at 915MHz
- **Blue:** Transmit success at 2400MHz
- **Purple:** Receive success at 2400MHz

## Usage

1. **Install Required Libraries:**
   - `Arduino.h`
   - `RadioLib.h`
   - `Wire.h`
   - `Adafruit_NeoPixel.h`

2. **Upload the Code:**
   - Upload the `Factory.ino` code to your Arduino development board.

3. **Open Serial Monitor:**
   - Open the serial monitor and set the baud rate to 115200.

4. **Switch Modes:**
   - Press the button to switch between different modes and observe the serial monitor output and LED status changes.

## Notes

- Ensure all hardware connections are correct.
- High power output (22 dBm) is only applicable for 868MHz and 915MHz bands.

---

This README provides a brief overview of the project, hardware setup, and usage instructions.
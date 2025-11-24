# Embedded Board Hardware Documentation

## Project Overview
This project is developed based on the ESP32 microcontroller, integrating dual LR1121 multi-band wireless communication modules, compatible with the ExpressLRS flight control platform. The system supports Sub-GHz/1.9GHz/2.4GHz multi-band communication, suitable for IoT application scenarios such as remote controller RF modules and data links.

## Hardware Configuration
| Module        | Model      | Key Features                              |
|---------------|------------|-------------------------------------------|
| Main MCU      | ESP32      | Dual-core processor, Wi-Fi/Bluetooth, 34x GPIO |
| Radio Module  | LR1121 x2  | Sub-GHz+1.9GHz/2.4GHz dual-band, LoRa®/FSK |
| Status LED    | LED        | GPIO5 controlled status indicator         |

## Pin Assignment
| Module         | Signal   | ESP32 Pin | Description          |
|----------------|----------|-----------|----------------------|
| **LR1121-1**   | MISO     | 33        | SPI data input       |
|                | MOSI     | 32        | SPI data output      |
|                | SCK      | 25        | SPI clock            |
|                | CS       | 27        | Chip select          |
|                | DIO9     | 37        | Interrupt signal     |
|                | RST      | 26        | Module reset         |
|                | BUSY     | 36        | Status output        |
| **LR1121-2**   | MISO     | 33        | SPI data input       |
|                | MOSI     | 32        | SPI data output      |
|                | SCK      | 25        | SPI clock            |
|                | CS       | 13        | Chip select          |
|                | DIO9     | 34        | Interrupt signal     |
|                | RST      | 21        | Module reset         |
|                | BUSY     | 39        | Status output        |
| **AT2401 IF**  | TX1      | 14        | RF switch control    |
|                | TX2      | 15        | RF switch control    |
|                | RX1      | 10        | RF switch control    |
|                | RX2      | 9         | RF switch control    |
| **Status LED** | LED      | 5         | SPI data input       |

## Functional Description
1. **Dual-module Architecture**: Two LR1121 modules share SPI bus (SCK/MOSI/MISO) for concurrent dual-band communication
2. **Hardware Isolation**: Independent CS/RESET/BUSY signals for each LR1121 ensure interference-free operation
4. **Status Monitoring**: DIO9 receives module interrupts, BUSY reflects real-time module status

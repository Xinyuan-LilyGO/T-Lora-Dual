# T-ELRS Documentation

## Project Overview
This project is developed based on the ESP32 (PICO-D4) microcontroller, integrating dual LR1121 multi-band wireless communication modules, and is compatible with the ExpressLRS flight control platform. The system supports Sub-GHz/1.9GHz/2.4GHz multi-band communication, suitable for remote controller RF modules, data links, and other IoT application scenarios.

## Hardware Configuration
| Module      | Model      | Main Features                                 |
| ----------- | ---------- | --------------------------------------------- |
| Main MCU    | ESP32      | Dual-core processor, supports Wi-Fi/Bluetooth, 34x GPIO pins |
| Wireless    | LR1121 x2  | Sub-GHz+1.9GHz/2.4GHz dual-band, LoRa®/FSK    |
| Status LED  | LED        | Controlled by GPIO5, indicates working status |

## Pin Assignment Table
| Functional Module   | Signal Name | ESP32 Pin | Description         |
| ------------------- | ----------- | --------- | ------------------- |
| **LR1121-1**        | MISO        | 33        | SPI Data In         |
|                     | MOSI        | 32        | SPI Data Out        |
|                     | SCK         | 25        | SPI Clock           |
|                     | CS          | 27        | Chip Select         |
|                     | DIO9        | 37        | Interrupt Signal    |
|                     | RST         | 26        | Module Reset        |
|                     | BUSY        | 36        | Status Output       |
| **LR1121-2**        | MISO        | 33        | SPI Data In         |
|                     | MOSI        | 32        | SPI Data Out        |
|                     | SCK         | 25        | SPI Clock           |
|                     | CS          | 13        | Chip Select         |
|                     | DIO9        | 34        | Interrupt Signal    |
|                     | RST         | 21        | Module Reset        |
|                     | BUSY        | 39        | Status Output       |
| **AT2401 Interface**| TX1         | 14        | RF Switch Control   |
|                     | TX2         | 15        | RF Switch Control   |
|                     | RX1         | 10        | RF Switch Control   |
|                     | RX2         | 9         | RF Switch Control   |
| **Status LED**      | LED         | 5         | SPI Data In         |

## Function Description
1. **Dual Module Architecture**: Two LR1121 modules are connected via a shared SPI bus (SCK/MOSI/MISO) to achieve concurrent dual-band communication.
2. **Hardware Isolation**: Each LR1121 module is independently configured with CS/RESET/BUSY signals to ensure non-interfering communication.
3. **Status Monitoring**: The DIO9 pin is used to receive module interrupt signals, and the BUSY pin reflects the module's working status in real time.

## Directory Structure
```text
├── ExpressLRS/                # ExpressLRS related code
│   └── src/                   # Source code
│       ├── user_defines       # ExpressLRS configuration
│       ├── hardware/          # Hardware directory
│       │   ├── RX/            # ExpressLRS receiver hardware IO mapping (T-ELRS LR1121 True Diversity.json)
│       │   ├── TX/            # ExpressLRS transmitter hardware IO mapping
│       │   └── targets.json   # ExpressLRS hardware IO build target selection (choose 1.BAYCKRC 900/2400 Dual Band Gemini RX)
│       └── lib/               # ExpressLRS dependencies
├── T-ELRS/                    # T-ELRS related code
│   ├── src/                   # Source code
│   ├── examples/              # Example code
│   ├── firmware/              # Firmware
│   └── hardware/              # Schematics
└── README.md                  # English documentation
└── README_CN.md               # Chinese documentation
```


For more in-depth information, please refer to [ExpressLRS](https://www.expresslrs.org/quick-start/getting-started/)

## Flashing
1. Before flashing, ensure ESP32 is in download mode (hold BOOT button, press RESET, release RESET).
2. Use ESP Flash Download Tool (Windows).
3. Check if the serial port driver is installed correctly.
4. If flashing fails, try lowering the baud rate or changing the USB cable.
5. After flashing, press RESET to restart the device.

## Compilation
### ExpressLRS Compilation (PlatformIO only)
1. Open the src subdirectory under ExpressLRS in VSCode.
2. Select the device model first ![](./image/image.png)![](./image/image1.png)
3. Then click build and upload. The first build may require downloading files and could be slow.
![](./image/image2.png)

### T-ELRS Compilation
#### PlatformIO
1. Open the T-ELRS directory in VSCode, then open the platformio file and uncomment the example to compile ![alt text](./image/image3.png)
2. Then click build and upload
![](./image/image2.png)

#### Arduino IDE
1. Move the lib directory under T-ELRS to the Arduino project libraries directory
![alt text](./image/image4.png)
2. Open the example under the examples directory with Arduino IDE
3. Select the chip model and default configuration
![alt text](./image/image5.png)
![alt text](./image/image6.png)
4. Click build and upload
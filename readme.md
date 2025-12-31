# T-ELRS Documentation
## Project Overview
This project is developed based on the ESP32(PICO-D4) microcontroller, integrating two LR1121 multi-band wireless communication modules, compatible with the ExpressLRS flight control platform. The system supports multi-band communication in Sub-GHz/1.9GHz/2.4GHz, suitable for IoT application scenarios such as remote control, data links, etc.

## Hardware Configuration
| Module           | Model     | Main Characteristics                                              |
| ---------------- | --------- | ----------------------------------------------------------------- |
| Main MCU         | ESP32     | Dual-core processor, supports Wi-Fi/Bluetooth, 34 GPIO interfaces |
| Wireless Module  | LR1121 x2 | Sub-GHz+1.9GHz/2.4GHz dual bands, LoRa®/FSK                       |
| Status Indicator | LED       | Controlled by GPIO5, indicates working status                     |

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

## Function Description
1. **Dual Module Architecture**: Two LR1121 modules are connected through a shared SPI bus (SCK/MOSI/MISO) to achieve concurrent communication in multi-bands.
2. **Hardware Isolation**: Each LR1121 module is independently configured with CS/RESET/BUSY signals to ensure that communications do not interfere with each other.
3. **Status Monitoring**: The DIO9 pin is used to receive interrupt signals from the module, and the BUSY pin reflects the real-time working status of the module.

```c
// RF switch mode table
static const Module::RfSwitchMode_t rfswitch_table[] = {
    // mode               DIO5,  DIO6, DIO7, DIO8
    {LR11x0::MODE_STBY,  { LOW,  LOW,  LOW,  LOW  }},
    {LR11x0::MODE_RX,    { LOW,  HIGH, HIGH, LOW  }},
    {LR11x0::MODE_TX,    { HIGH, LOW,  LOW,  LOW  }},
    {LR11x0::MODE_TX_HP, { HIGH, LOW,  LOW,  LOW  }},
    {LR11x0::MODE_TX_HF, { LOW,  LOW,  LOW,  HIGH }},
    {LR11x0::MODE_GNSS,  { LOW,  LOW,  LOW,  LOW  }},
    {LR11x0::MODE_WIFI,  { LOW,  LOW,  LOW,  LOW  }},
    END_OF_MODE_TABLE,
};
```

```c
├── ExpressLRS/                 # ExpressLRS related code directory
│   └── src/                    # Source code directory
│       ├── user_defines        # ExpressLRS configuration
│       ├── hardware/           # Hardware directory
│       │   ├── RX/              # ExpressLRS receiver hardware IO mapping file (T-ELRS LR1121 True Diversity.json)
│       │   ├── TX/              # ExpressLRS transmitter hardware IO mapping file
│       │   └── targets.json     # ExpressLRS hardware IO mapping compilation target file (compilation selection 1.BAYCKRC 900/2400 Dual Band Gemini RX)
│       └── lib/                # ExpressLRS dependent library
├── T-ELRS/                     # T-ELRS related code directory
│   ├── src/                    # Source code directory
│   ├── examples/               # Example code
│   ├── firmware/               # Firmware directory
│   └── hardware/               # Hardware schematic
└── README.md                   # English project description file
└── README_CN.md                # Chinese project description file
```

If you need to learn more, please visit [ExpressLRS official website](https://www.expresslrs.org/quick-start/getting-started/)

## Flashing
1. Ensure the ESP32 is in download mode before flashing (hold down the BOOT button, press RESET, then release RESET).
2. Use ESP Flash Download Tool (Windows).
3. Check if the serial port driver is properly installed.
4. If flashing fails, try reducing the baud rate or replacing the USB cable.
5. Restart the device by pressing the RESET button after flashing is complete.

## Compilation
### T-ELRS Compilation
#### Platformio
1. Open the T-ELRS directory in vscode, then open the platformio file and uncomment the routine you want to compile ![alt text](./image/image3.png)
2. Click compile and upload.
![](./image/image2.png)

#### Arduino IDE
1. Move the lib directory under the T-ELRS directory to the Arduino project dependent library directory.
![](./image/image4.png)
2. Open the routine in the examples directory with Arduino IDE.
3. Select the chip model, select the default configuration.
![](./image/image5.png)
![](./image/image6.png)
4. Click compile and upload.

### Firmware ExpressLRS Compilation (only Platformio)
1. Open the src subdirectory under the ExpressLRS directory in vscode.
2. First, select the device model. ![](./image/image.png)![](./image/image1.png)
3. Click compile and upload, the first compilation requires downloading files and may take a little longer.
![](./image/image2.png)

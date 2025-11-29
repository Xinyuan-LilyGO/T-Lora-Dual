# T-ELRS说明文档

## 项目概述
本项目基于ESP32(PICO-D4) 微控制器开发，集成双LR1121多频段无线通信模块，适配ExpressLRS飞控平台。系统支持Sub-GHz/1.9GHz/2.4GHz多频段通信，适用于遥控器高频头、数据链路等物联网应用场景。

## 硬件配置
| 模块     | 型号      | 主要特性                                 |
| -------- | --------- | ---------------------------------------- |
| 主控MCU  | ESP32     | 双核处理器，支持Wi-Fi/蓝牙，34x GPIO接口 |
| 无线模块 | LR1121 x2 | Sub-GHz+1.9GHz/2.4GHz双频段，LoRa®/FSK   |
| 状态指示 | LED       | GPIO5控制，工作状态指示                  |

## 引脚分配表
| 功能模块       | 信号名称 | ESP32引脚 | 说明         |
| -------------- | -------- | --------- | ------------ |
| **LR1121-1**   | MISO     | 33        | SPI数据输入  |
|                | MOSI     | 32        | SPI数据输出  |
|                | SCK      | 25        | SPI时钟      |
|                | CS       | 27        | 芯片选择     |
|                | DIO9     | 37        | 中断信号     |
|                | RST      | 26        | 模块复位     |
|                | BUSY     | 36        | 状态输出     |
| **LR1121-2**   | MISO     | 33        | SPI数据输入  |
|                | MOSI     | 32        | SPI数据输出  |
|                | SCK      | 25        | SPI时钟      |
|                | CS       | 13        | 芯片选择     |
|                | DIO9     | 34        | 中断信号     |
|                | RST      | 21        | 模块复位     |
|                | BUSY     | 39        | 状态输出     |
| **AT2401接口** | TX1      | 14        | 射频开关控制 |
|                | TX2      | 15        | 射频开关控制 |
|                | RX1      | 10        | 射频开关控制 |
|                | RX2      | 9         | 射频开关控制 |
| **状态指示**   | LED      | 5         | SPI数据输入  |

## 功能说明
1. **双模块架构**：通过共享SPI总线（SCK/MOSI/MISO）连接两个LR1121模块，实现双频段并发通信
2. **硬件隔离**：每个LR1121模块独立配置CS/RESET/BUSY信号，确保通信互不干扰
3. **状态监控**：DIO9引脚用于接收模块中断信号，BUSY引脚实时反映模块工作状态

## 文件目录说明
## 文件目录说明
├── ExpressLRS/                 # ExpressLRS相关代码目录
│   └── src/                    # 源代码目录
│       ├── user_defines        # ExpressLRS配置
│       ├── hardware/           # 硬件目录
│       │   ├── RX/              # ExpressLRS 接收机硬件IO对应文件(T-ELRS LR1121 True Diversity.json)
│       │   ├── TX/              # ExpressLRS 发射机硬件IO对应文件
│       │   └── targets.json     # ExpressLRS 选择硬件IO编译目标文件(编译选择 1.BAYCKRC 900/2400 Dual Band Gemini RX)
│       └── lib/                # ExpressLRS 依赖库
├── T-ELRS/                     # T-ELRS相关代码目录
│   ├── src/                    # 源代码目录
│   ├── examples/               # 示例代码
│   ├── firmware/               # 固件目录
│   └── hardware/               # 硬件原理图
└── README.md                   # 英文项目说明文件
└── README_CN.md                # 中文项目说明文件


若需深入了解，请查看[ExpressLRS 官网](https://www.expresslrs.org/quick-start/getting-started/)

## 烧录
1. 烧录前确保ESP32进入下载模式（需要持续按住BOOT按钮，再按RESET再松开RESET）
2. 使用ESP Flash Download Tool（Windows）
3. 检查串口驱动是否正常安装
4. 如遇烧录失败，尝试降低波特率或更换USB线缆
5. 烧录完成后按RESET按钮重启设备

## 编译
### ExpressLRS编译(只能Platfromio编译)
1. 用vscode打开ExpressLRS目录下的src子目录
2. 先选择设备型号![](./image/image.png)![](./image/image1.png)
3. 再点击编译上传，第一次编译需下载文件，可能会有点慢
![](./image/image2.png)

### T-ELRS编译
#### Platfromio
1. 用vscode打开T-ELRS目录,再打开platformio文件，取消注释要编译的例程![alt text](./image/image3.png)
2. 再点击编译上传
![](./image/image2.png)

#### Arduino IDE
1. 把T-ELR目录下的lib目录移动到Arduino项目依赖库目录下
![alt text](./image/image4.png)
2. 用Arduino IDE打开examples目录下的例程
3. 选择芯片型号，选择默认配置
![alt text](./image/image5.png)
![alt text](./image/image6.png)
4. 点击编译上传
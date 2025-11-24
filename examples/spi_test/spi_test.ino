#include "Arduino.h"
#include "SPI.h"

#define RADIO_MISO 33
#define RADIO_MOSI 32
#define RADIO_SCK 25

#define RADIO_CS1_PIN 27
#define RADIO_DIO9_1_PIN 37
#define RADIO_RST_1_PIN 26
#define RADIO_BUSY_1_PIN 36

#define LR1121_REG_VERSION 0x00 // 示例：版本寄存器
#define SPI_READ_FLAG 0x80      // 读操作标志位
#define LR11XX_SYSTEM_GET_VERSION_OC  0x0101


SPIClass hspi = SPIClass(HSPI);
uint8_t readRegister(uint8_t reg);

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("SPI Test");
    
    // 初始化SPI
    hspi.begin(RADIO_SCK, RADIO_MISO, RADIO_MOSI, RADIO_CS1_PIN);
    pinMode(RADIO_CS1_PIN, OUTPUT);
    digitalWrite(RADIO_CS1_PIN, HIGH);
    // 测试寄存器读取
    uint8_t version = readRegister(LR11XX_SYSTEM_GET_VERSION_OC);
    Serial.printf("[SPI] Version Register: 0x%02X\n", version);
}

void loop()
{
    delay(1000);
}


uint8_t readRegister(uint8_t reg) {
    uint8_t value = 0;
    
    digitalWrite(RADIO_CS1_PIN, LOW);
    
    // 发送读命令：寄存器地址 | 读标志位
    hspi.transfer(reg | SPI_READ_FLAG);
    
    // 读取返回值
    value = hspi.transfer(0xFF);
    
    digitalWrite(RADIO_CS1_PIN, HIGH);
    
    return value;
  }
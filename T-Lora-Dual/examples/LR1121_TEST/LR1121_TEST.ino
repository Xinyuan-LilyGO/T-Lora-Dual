#include <Arduino.h>
#include <RadioLib.h>
#include <Wire.h>
#include "esp_sleep.h"
#include <Adafruit_NeoPixel.h>

#define RADIO_MISO 33
#define RADIO_MOSI 32
#define RADIO_SCK 25

#define RADIO_CS1_PIN 27
#define RADIO_DIO9_1_PIN 37
#define RADIO_RST_1_PIN 26
#define RADIO_BUSY_1_PIN 36

#define RADIO_CS2_PIN 13
#define RADIO_DIO9_2_PIN 34
#define RADIO_RST_2_PIN 21
#define RADIO_BUSY_2_PIN 39

// The maximum power of LR1121 Sub 1G band can only be set to 22 dBm
#define CONFIG_RADIO_FREQ 868.0
#define CONFIG_RADIO_BW 125.0
#define CONFIG_RADIO_SF 7
#define CONFIG_RADIO_CR 5
#define CONFIG_RADIO_SYSN 0x12
#define CONFIG_RADIO_OUTPUT_POWER 13
#define CONFIG_RADIO_PERLEN 12

#define BOOT_PIN 0

#define NUM_LEDS 1
#define LED_PIN 5
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define USING_LR1121
LR1121 radio_1 = new Module(RADIO_CS1_PIN, RADIO_DIO9_1_PIN, RADIO_RST_1_PIN, RADIO_BUSY_1_PIN);
bool radio_1_transmittedFlag = false;
bool radio_1_receivedFlag = false;
int radio_1_transmissionState = RADIOLIB_ERR_NONE;

LR1121 radio_2 = new Module(RADIO_CS2_PIN, RADIO_DIO9_2_PIN, RADIO_RST_2_PIN, RADIO_BUSY_2_PIN);
bool radio_2_transmittedFlag = false;
bool radio_2_receivedFlag = false;
int radio_2_transmissionState = RADIOLIB_ERR_NONE;

volatile bool buttonPressedFlag = false;
enum RadioButtonState_t
{
  BUTTON_TX_868 = 0,
  BUTTON_TX_915,
  BUTTON_TX_2400,
  BUTTON_RX_868,
  BUTTON_RX_915,
  BUTTON_RX_2400,
  BUTTON_MAX, // must be last
};
uint8_t RadioButtonState = BUTTON_TX_868;
float freq = CONFIG_RADIO_FREQ;

typedef enum
{
  RF_MODE_STANDBY,
  RF_MODE_TX,
  RF_MODE_RX,
} RF_Mode_t;
RF_Mode_t RF_Mode = RF_MODE_TX;

#if defined(USING_LR1121)

static const uint32_t rfSwitchPins[] = {
    RADIOLIB_LR11X0_DIO5,
    RADIOLIB_LR11X0_DIO6,
    RADIOLIB_LR11X0_DIO7,
    RADIOLIB_LR11X0_DIO8,
    RADIOLIB_NC};

static const Module::RfSwitchMode_t rfswitch_table[] = {
    // mode              DIO5,DIO6,DIO_HF_RX,DIO_HF_TX
    {LR11x0::MODE_STBY, {LOW, LOW, LOW, LOW}},
    {LR11x0::MODE_RX, {LOW, HIGH, HIGH, LOW}},
    {LR11x0::MODE_TX, {HIGH, LOW, LOW, LOW}},
    {LR11x0::MODE_TX_HP, {HIGH, LOW, LOW, LOW}},
    {LR11x0::MODE_TX_HF, {LOW, LOW, LOW, HIGH}},
    {LR11x0::MODE_GNSS, {LOW, LOW, LOW, LOW}},
    {LR11x0::MODE_WIFI, {LOW, LOW, LOW, LOW}},
    END_OF_MODE_TABLE,
};
#endif

void setRainbowColor(int pixelIndex, int colorIndex);
void radio_config(LR1121 &radio, RF_Mode_t rf_mode, float freq, float bw, uint8_t sf, uint8_t cr,
                  uint8_t sysn, int8_t outputPower, uint16_t perlen);

void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  int state;
  static uint8_t frameCounter = 0;
  switch (eventType)
  {
  case AceButton::kEventClicked:
    Serial.println("Clicked");
    break;

  case AceButton::kEventLongPressed:
    Serial.println("Long Pressed");
    break;
  }
}

void radio_config(LR1121 &radio, RF_Mode_t rf_mode, float freq, float bw, uint8_t sf, uint8_t cr,
                  uint8_t sysn, int8_t outputPower, uint16_t perlen)
{
  if (radio.setFrequency(freq) == RADIOLIB_ERR_INVALID_FREQUENCY)
  {
    Serial.println(F("Selected frequency is invalid for this module!"));
    while (true)
      ;
  }

  /*
   *   Sets LoRa link bandwidth.
   *   SX1278/SX1276 : Allowed values are 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250 and 500 kHz. Only available in %LoRa mode.
   *   SX1268/SX1262 : Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
   *   SX1280        : Allowed values are 203.125, 406.25, 812.5 and 1625.0 kHz.
   *   LR1121        : Allowed values are 62.5, 125.0, 250.0 and 500.0 kHz.
   * * * */
  if (radio.setBandwidth(bw) == RADIOLIB_ERR_INVALID_BANDWIDTH)
  {
    Serial.println(F("Selected bandwidth is invalid for this module!"));
    while (true)
      ;
  }

  /*
   * Sets LoRa link spreading factor.
   * SX1278/SX1276 :  Allowed values range from 6 to 12. Only available in LoRa mode.
   * SX1262        :  Allowed values range from 5 to 12.
   * SX1280        :  Allowed values range from 5 to 12.
   * LR1121        :  Allowed values range from 5 to 12.
   * * * */
  if (radio.setSpreadingFactor(sf) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR)
  {
    Serial.println(F("Selected spreading factor is invalid for this module!"));
    while (true)
      ;
  }

  /*
   * Sets LoRa coding rate denominator.
   * SX1278/SX1276/SX1268/SX1262 : Allowed values range from 5 to 8. Only available in LoRa mode.
   * SX1280        :  Allowed values range from 5 to 8.
   * LR1121        :  Allowed values range from 5 to 8.
   * * * */
  if (radio.setCodingRate(cr) == RADIOLIB_ERR_INVALID_CODING_RATE)
  {
    Serial.println(F("Selected coding rate is invalid for this module!"));
    while (true)
      ;
  }

  /*
   * Sets transmission output power.
   * SX1278/SX1276 :  Allowed values range from -3 to 15 dBm (RFO pin) or +2 to +17 dBm (PA_BOOST pin). High power +20 dBm operation is also supported, on the PA_BOOST pin. Defaults to PA_BOOST.
   * SX1262        :  Allowed values are in range from -9 to 22 dBm. This method is virtual to allow override from the SX1261 class.
   * SX1268        :  Allowed values are in range from -9 to 22 dBm.
   * SX1280        :  Allowed values are in range from -18 to 13 dBm. PA Version range : -18 ~ 3dBm
   * LR1121        :  Allowed values are in range from -17 to 22 dBm (high-power PA) or -18 to 13 dBm (High-frequency PA)
   * * * */
  if (radio.setOutputPower(outputPower) == RADIOLIB_ERR_INVALID_OUTPUT_POWER)
  {
    Serial.println(F("Selected output power is invalid for this module!"));
    while (true)
      ;
  }

  /*
   * Sets preamble length for LoRa or FSK modem.
   * SX1278/SX1276 : Allowed values range from 6 to 65535 in %LoRa mode or 0 to 65535 in FSK mode.
   * SX1262/SX1268 : Allowed values range from 1 to 65535.
   * SX1280        : Allowed values range from 1 to 65535.
   * LR1121        : Allowed values range from 1 to 65535.
   * * */
  if (radio.setPreambleLength(perlen) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH)
  {
    Serial.println(F("Selected preamble length is invalid for this module!"));
    while (true)
      ;
  }

  if (radio.setSyncWord(sysn) == RADIOLIB_ERR_INVALID_SYNC_WORD)
  {
    Serial.println(F("Selected sync word is invalid for this module!"));
    while (true)
      ;
  }

  radio.invertIQ(false);
  radio.setTCXO(3.3); // BAYCKRC board has no TCXO

  int state;
  if (rf_mode == RF_MODE_TX)
  {
    radio.startTransmit("T-Lora Dual!");
  }
  else if (rf_mode == RF_MODE_RX)
  {
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE)
    {
      Serial.println(F("success!"));
    }
    else
    {
      Serial.print(F("failed, code "));
      Serial.println(state);
      while (true)
        ;
    }
  }
}

void setRainbowColor(int pixelIndex, int colorIndex)
{
  switch (colorIndex)
  {
  case 0: // 红色
    strip.setPixelColor(pixelIndex, 255, 0, 0);
    break;
  case 1: // 橙色
    strip.setPixelColor(pixelIndex, 255, 127, 0);
    break;
  case 2: // 黄色
    strip.setPixelColor(pixelIndex, 255, 255, 0);
    break;
  case 3: // 绿色
    strip.setPixelColor(pixelIndex, 0, 255, 0);
    break;
  case 4: // 青色
    strip.setPixelColor(pixelIndex, 0, 255, 255);
    break;
  case 5: // 蓝色
    strip.setPixelColor(pixelIndex, 0, 0, 255);
    break;
  case 6: // 紫色
    strip.setPixelColor(pixelIndex, 139, 0, 255);
    break;
  }
  strip.show();
}

void radio_TX(LR1121 &radio, uint8_t number)
{
  int16_t err;
  radio.finishTransmit();

  static int transmissionCounter = 0;
  String str = "T-Lora Dual! Radio_" + String(number) + " #" + String(transmissionCounter++);
  Serial.printf("%s\n", str.c_str());

  err = radio.startTransmit(str);

  if (err == RADIOLIB_ERR_NONE)
  {
    if (freq == 868.0)
    {
      setRainbowColor(0, 1);
    }
    else if (freq == 915.0)
    {
      setRainbowColor(0, 3);
    }
    else if (freq == 2400.0)
    {
      setRainbowColor(0, 5);
    }
  }
  else
  {
    setRainbowColor(0, 0);
    Serial.print(F("failed, code "));
    Serial.println(err);
  }
}

void radio_RX(LR1121 &radio, uint8_t number)
{
  String str;
  int state = radio.readData(str);

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(str);
    Serial.printf("RSSI_%d: %.2f dbm\n", number, radio.getRSSI());
  }
  else
  {
    setRainbowColor(0, 0);
    Serial.print(F("failed, code "));
    Serial.println(state);
  }

  if (freq == 868.0)
  {
    setRainbowColor(0, 2);
  }
  else if (freq == 915.0)
  {
    setRainbowColor(0, 4);
  }
  else if (freq == 2400.0)
  {
    setRainbowColor(0, 6);
  }
}

#define DEBOUNCE_DELAY 50           // 消抖延时，单位毫秒
unsigned long lastDebounceTime = 0; // 记录上次按钮状态变化的时间
int lastButtonState = HIGH;         // 记录上次的按钮状态
void buttonPressed()
{
  unsigned long currentTime = millis();

  // 检查是否已经过了消抖延时
  if ((currentTime - lastDebounceTime) > DEBOUNCE_DELAY)
  {
    // 读取当前按钮状态
    int buttonState = digitalRead(BOOT_PIN);

    // 如果按钮状态确实发生了变化
    if (buttonState != lastButtonState && buttonState == LOW)
    {
      buttonPressedFlag = true;
      RadioButtonState++;
      RadioButtonState %= BUTTON_MAX;
      Serial.printf("Button Pressed! RadioButtonState: %d\n", RadioButtonState);
      lastDebounceTime = currentTime;
    }
    lastButtonState = buttonState;
  }
}

void radio_tx1_setFlag(void)
{
  radio_1_transmittedFlag = true;
}

void radio_tx2_setFlag(void)
{
  radio_2_transmittedFlag = true;
}

void radio_rx1_setFlag(void)
{
  radio_1_receivedFlag = true;
}

void radio_rx2_setFlag(void)
{
  radio_2_receivedFlag = true;
}

void setup()
{
  pinMode(BOOT_PIN, INPUT_PULLUP);
  attachInterrupt(BOOT_PIN, buttonPressed, FALLING);

  strip.begin(); // 初始化
  strip.show();  // 先关掉所有灯（防止上电闪屏）

  strip.setBrightness(50); // 设置亮度
  strip.setPixelColor(1, 255, 255, 255);
  strip.show();

  pinMode(RADIO_CS1_PIN, OUTPUT);
  digitalWrite(RADIO_CS1_PIN, HIGH);
  pinMode(RADIO_CS2_PIN, OUTPUT);
  digitalWrite(RADIO_CS2_PIN, HIGH);

  Serial.begin(115200, SERIAL_8N1, 3, 1);
  SPI.begin(RADIO_SCK, RADIO_MISO, RADIO_MOSI, RADIO_CS1_PIN);

  delay(1000);

  Serial.print(F("[Radio] Initializing ... "));

  int16_t state = radio_1.begin();

  LR11x0VersionInfo_t info;
  state = radio_1.getVersionInfo(&info);
  Serial.printf("LR1121_radio_1 Version: %02X.%02X.%02X\n", info.hardware, info.device, info.fwMajor);
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed!  "));
    Serial.println(state);
  }
  radio_1.setRfSwitchTable(rfSwitchPins, rfswitch_table);

  uint8_t outpower = 0;
  if (freq >= 1900 && freq <= 2500)
  {
    outpower = 13;
  }
  else
  {
    outpower = 22;
  }
  if (RF_Mode == RF_MODE_TX)
  {
    radio_1.setPacketSentAction(radio_tx1_setFlag);
  }
  else if (RF_Mode == RF_MODE_RX)
  {
    radio_1.setPacketReceivedAction(radio_rx1_setFlag);
  }
  radio_config(radio_1, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
               CONFIG_RADIO_SYSN, outpower, CONFIG_RADIO_PERLEN);

  /********radio2*********/
  Serial.print(F("[Radio] Initializing ... "));
  state = radio_2.begin();
  state = radio_2.getVersionInfo(&info);
  Serial.printf("LR1121_radio_2 Version: %02X.%02X.%02X\n", info.hardware, info.device, info.fwMajor);
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed!  "));
    Serial.println(state);
  }
  radio_2.setRfSwitchTable(rfSwitchPins, rfswitch_table);

  if (RF_Mode == RF_MODE_TX)
  {
    radio_2.setPacketSentAction(radio_tx2_setFlag);
  }
  else if (RF_Mode == RF_MODE_RX)
  {
    radio_2.setPacketReceivedAction(radio_rx2_setFlag);
  }
  radio_config(radio_2, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
               CONFIG_RADIO_SYSN, outpower, CONFIG_RADIO_PERLEN);
}

static unsigned long lastPress = 0;
void loop()
{
  if (millis() - lastPress > 1000)
  {
    lastPress = millis();

    if (buttonPressedFlag)
    {
      buttonPressedFlag = false;
      switch (RadioButtonState)
      {
      case BUTTON_TX_868:
        RF_Mode = RF_MODE_TX;
        freq = 868.0;
        Serial.printf("[Radio] RF_Mode: %d,%lf\n", RF_Mode, freq);
        if (RF_Mode == RF_MODE_TX)
        {
          radio_1.setPacketSentAction(radio_tx1_setFlag);
          radio_2.setPacketSentAction(radio_tx2_setFlag);
        }
        else if (RF_Mode == RF_MODE_RX)
        {
          radio_1.setPacketReceivedAction(radio_rx1_setFlag);
          radio_2.setPacketReceivedAction(radio_rx2_setFlag);
        }
        radio_config(radio_1, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 22, CONFIG_RADIO_PERLEN);
        radio_config(radio_2, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 22, CONFIG_RADIO_PERLEN);
        break;
      case BUTTON_TX_915:
        RF_Mode = RF_MODE_TX;
        freq = 915.0;
        Serial.printf("[Radio] RF_Mode: %d,%lf\n", RF_Mode, freq);
        if (RF_Mode == RF_MODE_TX)
        {
          radio_1.setPacketSentAction(radio_tx1_setFlag);
          radio_2.setPacketSentAction(radio_tx2_setFlag);
        }
        else if (RF_Mode == RF_MODE_RX)
        {
          radio_1.setPacketReceivedAction(radio_rx1_setFlag);
          radio_2.setPacketReceivedAction(radio_rx2_setFlag);
        }
        radio_config(radio_1, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 22, CONFIG_RADIO_PERLEN);
        radio_config(radio_2, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 22, CONFIG_RADIO_PERLEN);
        break;
      case BUTTON_TX_2400:
        RF_Mode = RF_MODE_TX;
        freq = 2400.0;
        Serial.printf("[Radio] RF_Mode: %d,%lf\n", RF_Mode, freq);
        if (RF_Mode == RF_MODE_TX)
        {
          radio_1.setPacketSentAction(radio_tx1_setFlag);
          radio_2.setPacketSentAction(radio_tx2_setFlag);
        }
        else if (RF_Mode == RF_MODE_RX)
        {
          radio_1.setPacketReceivedAction(radio_rx1_setFlag);
          radio_2.setPacketReceivedAction(radio_rx2_setFlag);
        }
        radio_config(radio_1, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 13, CONFIG_RADIO_PERLEN);
        radio_config(radio_2, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 13, CONFIG_RADIO_PERLEN);
        break;
      case BUTTON_RX_868:
        RF_Mode = RF_MODE_RX;
        freq = 868.0;
        Serial.printf("[Radio] RF_Mode: %d,%lf\n", RF_Mode, freq);
        if (RF_Mode == RF_MODE_TX)
        {
          radio_1.setPacketSentAction(radio_tx1_setFlag);
          radio_2.setPacketSentAction(radio_tx2_setFlag);
        }
        else if (RF_Mode == RF_MODE_RX)
        {
          radio_1.setPacketReceivedAction(radio_rx1_setFlag);
          radio_2.setPacketReceivedAction(radio_rx2_setFlag);
        }
        radio_config(radio_1, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 22, CONFIG_RADIO_PERLEN);
        radio_config(radio_2, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 22, CONFIG_RADIO_PERLEN);
        setRainbowColor(0, 2);
        break;
      case BUTTON_RX_915:
        RF_Mode = RF_MODE_RX;
        freq = 915.0;
        Serial.printf("[Radio] RF_Mode: %d,%lf\n", RF_Mode, freq);
        if (RF_Mode == RF_MODE_TX)
        {
          radio_1.setPacketSentAction(radio_tx1_setFlag);
          radio_2.setPacketSentAction(radio_tx2_setFlag);
        }
        else if (RF_Mode == RF_MODE_RX)
        {
          radio_1.setPacketReceivedAction(radio_rx1_setFlag);
          radio_2.setPacketReceivedAction(radio_rx2_setFlag);
        }
        radio_config(radio_1, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 22, CONFIG_RADIO_PERLEN);
        radio_config(radio_2, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 22, CONFIG_RADIO_PERLEN);
        setRainbowColor(0, 4);
        break;
      case BUTTON_RX_2400:
        RF_Mode = RF_MODE_RX;
        freq = 2400.0;
        Serial.printf("[Radio] RF_Mode: %d,%lf\n", RF_Mode, freq);
        if (RF_Mode == RF_MODE_TX)
        {
          radio_1.setPacketSentAction(radio_tx1_setFlag);
          radio_2.setPacketSentAction(radio_tx2_setFlag);
        }
        else if (RF_Mode == RF_MODE_RX)
        {
          radio_1.setPacketReceivedAction(radio_rx1_setFlag);
          radio_2.setPacketReceivedAction(radio_rx2_setFlag);
        }
        radio_config(radio_1, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 13, CONFIG_RADIO_PERLEN);
        radio_config(radio_2, RF_Mode, freq, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR,
                     CONFIG_RADIO_SYSN, 13, CONFIG_RADIO_PERLEN);
        setRainbowColor(0, 6);
        break;
      }
    }

    static uint8_t rf_switch = 0;
    switch (RF_Mode)
    {
    case RF_MODE_TX:
      rf_switch++;
      rf_switch %= 2;
      if (radio_1_transmittedFlag && rf_switch == 0)
      {
        radio_1_transmittedFlag = false;
        radio_TX(radio_1, 1);
      }
      else if (radio_2_transmittedFlag && rf_switch == 1)
      {
        radio_2_transmittedFlag = false;
        radio_TX(radio_2, 2);
      }

      break;
    case RF_MODE_RX:
      if (radio_1_receivedFlag || radio_2_receivedFlag)
      {
        if (radio_1_receivedFlag)
        {
          radio_1_receivedFlag = false;
          radio_RX(radio_1, 1);
        }

        if (radio_2_receivedFlag)
        {
          radio_2_receivedFlag = false;
          radio_RX(radio_2, 2);
        }
      }
      break;
    }
  }
}
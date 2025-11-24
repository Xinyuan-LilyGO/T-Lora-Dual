#include <Arduino.h>
#include <RadioLib.h>
#include <Wire.h>
#include "esp_sleep.h"

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

#define RADIO_AT2401_TX_1 14
#define RADIO_AT2401_TX_2 15
#define RADIO_AT2401_RX_1 10
#define RADIO_AT2401_RX_2 9

// The maximum power of LR1121 Sub 1G band can only be set to 22 dBm
#define CONFIG_RADIO_FREQ 2400.0
#define CONFIG_RADIO_BW 250.0
#define CONFIG_RADIO_SF 7
#define CONFIG_RADIO_CR 5
#define CONFIG_RADIO_SYSN 0x34
#define CONFIG_RADIO_OUTPUT_POWER 13
#define CONFIG_RADIO_PERLEN 12

#define BOOT_PIN 0

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
enum RF_Mode_t
{
  RF_MODE_STANDBY,
  RF_MODE_TX,
  RF_MODE_RX,
};
int RF_Mode = RF_MODE_TX;

#if defined(USING_LR1121)
static const uint32_t rfswitch_dio_pins[] = {
    RADIOLIB_LR11X0_DIO5, RADIOLIB_LR11X0_DIO6,
    RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC};

static const Module::RfSwitchMode_t rfswitch_table[] = {
    // mode              DIO5  DIO6
    {LR11x0::MODE_STBY, {LOW, LOW}},
    {LR11x0::MODE_RX, {LOW, HIGH}},
    {LR11x0::MODE_TX, {LOW, LOW}},
    {LR11x0::MODE_TX_HP, {LOW, LOW}},
    {LR11x0::MODE_TX_HF, {LOW, LOW}},
    {LR11x0::MODE_GNSS, {LOW, LOW}},
    {LR11x0::MODE_WIFI, {LOW, LOW}},
    END_OF_MODE_TABLE,
};
#endif

void radio_config(LR1121 &radio)
{
  if (radio.setFrequency(CONFIG_RADIO_FREQ) == RADIOLIB_ERR_INVALID_FREQUENCY)
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
  if (radio.setBandwidth(CONFIG_RADIO_BW) == RADIOLIB_ERR_INVALID_BANDWIDTH)
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
  if (radio.setSpreadingFactor(CONFIG_RADIO_SF) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR)
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
  if (radio.setCodingRate(CONFIG_RADIO_CR) == RADIOLIB_ERR_INVALID_CODING_RATE)
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
  if (radio.setOutputPower(13) == RADIOLIB_ERR_INVALID_OUTPUT_POWER)
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
  if (radio.setPreambleLength(CONFIG_RADIO_PERLEN) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH)
  {
    Serial.println(F("Selected preamble length is invalid for this module!"));
    while (true)
      ;
  }

  if (radio.setSyncWord(CONFIG_RADIO_SYSN) == RADIOLIB_ERR_INVALID_SYNC_WORD)
  {
    Serial.println(F("Selected sync word is invalid for this module!"));
    while (true)
      ;
  }

  radio.invertIQ(false);
  // LR1121 TCXO Voltage 2.85~3.15V
  // radio.setTCXO(3.3); // BAYCKRC board has no TCXO
}

void radio_TX(LR1121 &radio, uint8_t number)
{
  radio.finishTransmit();
  delay(1000);

  static int transmissionCounter = 0;
  String str = "T-ELRS! Radio_" + String(number) + " #" + String(transmissionCounter++);
  Serial.printf("[Radio] Sending another packet ... %s\n", str.c_str());

  if ((CONFIG_RADIO_FREQ >= 1900.0 && CONFIG_RADIO_FREQ <= 2100.0) || (CONFIG_RADIO_FREQ >= 2400.0 && CONFIG_RADIO_FREQ <= 2500.0))
  {
    set_at2401_pins(RF_MODE_TX, number);
  }

  radio_1_transmissionState = radio.startTransmit(str);

  if (radio_1_transmissionState == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(radio_1_transmissionState);
  }
}

void radio_RX(LR1121 &radio, uint8_t number)
{
  // you can receive data as an Arduino String
  String str;
  // int state = radio.receive(str);
  int state = radio.readData(str);

  if ((CONFIG_RADIO_FREQ >= 1900.0 && CONFIG_RADIO_FREQ <= 2100.0) || (CONFIG_RADIO_FREQ >= 2400.0 && CONFIG_RADIO_FREQ <= 2500.0))
  {
    // set_at2401_pins(RF_MODE_STANDBY, number % 2 + 1);
    set_at2401_pins(RF_MODE_RX, number);
  }

  if (state == RADIOLIB_ERR_NONE)
  {
    // packet was successfully received
    // Serial.println(F("success!"));

    // print the data of the packet
    // Serial.printf("Data_%d:\t\t", number);
    Serial.println(str);

    // print the RSSI (Received Signal Strength Indicator)
    // of the last received packet
    Serial.printf("RSSI_%d:\t\t", number);
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    // print the SNR (Signal-to-Noise Ratio)
    // of the last received packet
    // Serial.printf("SNR_%d:\t\t", number);
    // Serial.print(radio.getSNR());
    // Serial.println(F(" dB"));
  }
  else if (state == RADIOLIB_ERR_CRC_MISMATCH)
  {
    // packet was received, but is malformed
    Serial.println(F("CRC error!"));
  }
  else
  {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}

void buttonPressed()
{
  buttonPressedFlag = true;
}

void set_at2401_pins(RF_Mode_t RF_Mode, uint8_t number)
{
  switch (RF_Mode)
  {
  case RF_MODE_STANDBY:
    if (number == 1)
    {
      pinMode(RADIO_AT2401_RX_1, OUTPUT);
      digitalWrite(RADIO_AT2401_RX_1, LOW);
      pinMode(RADIO_AT2401_TX_1, OUTPUT);
      digitalWrite(RADIO_AT2401_TX_1, LOW);
      return;
    }
    else if (number == 2)
    {
      pinMode(RADIO_AT2401_RX_2, OUTPUT);
      digitalWrite(RADIO_AT2401_RX_2, LOW);
      pinMode(RADIO_AT2401_TX_2, OUTPUT);
      digitalWrite(RADIO_AT2401_TX_2, LOW);
      return;
    }
    break;
  case RF_MODE_TX:
    if (number == 1)
    {
      pinMode(RADIO_AT2401_RX_1, OUTPUT);
      digitalWrite(RADIO_AT2401_RX_1, LOW);
      pinMode(RADIO_AT2401_TX_1, OUTPUT);
      digitalWrite(RADIO_AT2401_TX_1, HIGH);
      return;
    }
    else if (number == 2)
    {
      pinMode(RADIO_AT2401_RX_2, OUTPUT);
      digitalWrite(RADIO_AT2401_RX_2, LOW);
      pinMode(RADIO_AT2401_TX_2, OUTPUT);
      digitalWrite(RADIO_AT2401_TX_2, HIGH);
      return;
    }
    break;
  case RF_MODE_RX:
    if (number == 1)
    {
      pinMode(RADIO_AT2401_RX_1, OUTPUT);
      digitalWrite(RADIO_AT2401_RX_1, HIGH);
      pinMode(RADIO_AT2401_TX_1, OUTPUT);
      digitalWrite(RADIO_AT2401_TX_1, LOW);
      return;
    }
    else if (number == 2)
    {
      pinMode(RADIO_AT2401_RX_2, OUTPUT);
      digitalWrite(RADIO_AT2401_RX_2, HIGH);
      pinMode(RADIO_AT2401_TX_2, OUTPUT);
      digitalWrite(RADIO_AT2401_TX_2, LOW);
      return;
    }
    break;
  }
}

void radio_tx1_setFlag(void)
{
  radio_1_transmittedFlag = true;
  set_at2401_pins(RF_MODE_STANDBY, 1);
}

void radio_tx2_setFlag(void)
{
  radio_2_transmittedFlag = true;
  set_at2401_pins(RF_MODE_STANDBY, 2);
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

  set_at2401_pins(RF_MODE_STANDBY, 1);
  set_at2401_pins(RF_MODE_STANDBY, 2);

  pinMode(RADIO_CS1_PIN, OUTPUT);
  digitalWrite(RADIO_CS1_PIN, HIGH);
  pinMode(RADIO_CS2_PIN, OUTPUT);
  digitalWrite(RADIO_CS2_PIN, HIGH);

  Serial.begin(115200, SERIAL_8N1, 3, 1);
  SPI.begin(RADIO_SCK, RADIO_MISO, RADIO_MOSI, -1);

  delay(1000);

  Serial.print(F("[Radio] Initializing ... "));

  int16_t state = radio_1.begin();
  radio_1.setRfSwitchTable(rfswitch_dio_pins, rfswitch_table);
  radio_config(radio_1);

  LR11x0VersionInfo_t info;
  state = radio_1.getVersionInfo(&info);
  Serial.printf("LR1121 Version: %02X.%02X.%02X\n", info.hardware, info.device, info.fwMajor);

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed!  "));
    Serial.println(state);
  }

  if (RF_Mode == RF_MODE_TX)
  {
    radio_1.setPacketSentAction(radio_tx1_setFlag);
    Serial.print(F("[LR1110] Sending first packet ... "));
    radio_1_transmittedFlag = radio_1.startTransmit("T-ELRS!");

    /*******TEST*******/
    // set_at2401_pins(RF_MODE_TX, 1);
    // if(radio_1.startCW() != RADIOLIB_ERR_NONE)
    // {
    //   Serial.println(F("startCW failed!"));
    // }
  }
  else if (RF_Mode == RF_MODE_RX)
  {
    radio_1.setPacketReceivedAction(radio_rx1_setFlag);
    set_at2401_pins(RF_MODE_RX, 1);
    state = radio_1.startReceive();
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

  /********radio2*********/
  Serial.print(F("[Radio] Initializing ... "));
  state = radio_2.begin();
  radio_2.setRfSwitchTable(rfswitch_dio_pins, rfswitch_table);
  radio_config(radio_2);

  if (RF_Mode == RF_MODE_TX)
  {
    radio_2.setPacketSentAction(radio_tx2_setFlag);
    Serial.print(F("[LR1110] Sending first packet ... "));
    radio_2_transmittedFlag = radio_2.startTransmit("T-ELRS!");
  }
  else if (RF_Mode == RF_MODE_RX)
  {
    radio_2.setPacketReceivedAction(radio_rx2_setFlag);
    set_at2401_pins(RF_MODE_RX, 2);
    state = radio_2.startReceive();
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

void loop()
{
  switch (RF_Mode)
  {
  case RF_MODE_TX:
    if (radio_1_transmittedFlag)
    {
      radio_1_transmittedFlag = false;
      if (radio_1_transmissionState == RADIOLIB_ERR_NONE)
      {
        Serial.println(F("radio_1 transmission finished!"));
      }
      else
      {
        Serial.print(F("radio_1 failed, code "));
        Serial.println(radio_1_transmissionState);
      }
      radio_TX(radio_1, 1);
    }

    if (radio_2_transmittedFlag)
    {
      radio_2_transmittedFlag = false;
      if (radio_2_transmissionState == RADIOLIB_ERR_NONE)
      {
        Serial.println(F("radio_2 transmission finished!"));
      }
      else
      {
        Serial.print(F("radio_2 failed, code "));
        Serial.println(radio_2_transmissionState);
      }
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

  if (buttonPressedFlag)
  {
    buttonPressedFlag = false;
    int16_t state;
    static unsigned long lastPress = 0;
    if (millis() - lastPress > 200)
    {
      lastPress = millis();
      Serial.println(F("[Button] Pressed!"));
      RF_Mode = (RF_Mode == RF_MODE_TX) ? RF_MODE_RX : RF_MODE_TX;
      Serial.printf("[Radio] RF_Mode: %d\n", RF_Mode);

      if (RF_Mode == RF_MODE_TX)
      {
        radio_1.setPacketSentAction(radio_tx1_setFlag);
        Serial.print(F("[LR1110] Sending first packet ... "));
        radio_1_transmittedFlag = radio_1.startTransmit("T-ELRS!");

        radio_2.setPacketSentAction(radio_tx1_setFlag);
        Serial.print(F("[LR1110] Sending first packet ... "));
        radio_2_transmittedFlag = radio_2.startTransmit("T-ELRS!");
      }
      else if (RF_Mode == RF_MODE_RX)
      {
        radio_1.setPacketReceivedAction(radio_rx1_setFlag);
        set_at2401_pins(RF_MODE_RX, 1);
        state = radio_1.startReceive();
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

        radio_2.setPacketReceivedAction(radio_rx2_setFlag);
        set_at2401_pins(RF_MODE_RX, 2);
        state = radio_2.startReceive();
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
  }
}

#include <WiFi.h>

void setup() {
  Serial.begin(115200);

  // 设置WiFi模式为AP模式
  WiFi.mode(WIFI_AP);

  // 设置Wi-Fi发射功率为20 dBm（具体值可能需要根据你的ESP32型号和区域规定进行调整）
  bool result = WiFi.setTxPower(WIFI_POWER_19_5dBm);
  if (result) {
    Serial.println("Wi-Fi发射功率设置成功");
  } else {
    Serial.println("Wi-Fi发射功率设置失败");
  }

  // 获取当前的Wi-Fi发射功率
  int8_t txPower = WiFi.getTxPower();
  Serial.print("当前Wi-Fi发射功率: ");
  Serial.print(txPower);
  Serial.println(" dBm");

  // 配置并启动AP模式
  WiFi.softAP("ESP32_AP", "");

  Serial.println("AP模式启动成功");
  Serial.print("AP的IP地址: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // 主循环代码
  // 这里可以添加其他功能，比如处理客户端连接等
  Serial.println("wifi test running!");  
  delay(3000);
}

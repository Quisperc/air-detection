#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// ===== 用户配置 =====
const char* ssid = "杂鱼~♡没网的杂鱼~";      // WiFi名称
const char* password = "20ddh38xjsi.eea";         // WiFi密码
const char* targetIPStr = "110.41.143.68";  // 目标服务器IP（电脑）
const int targetPort = 9091;                 // 目标端口

// ===== 调试配置 =====
#define DEBUG_BAUDRATE   115200    // 调试串口波特率
#define DATA_TIMEOUT     30000     // STM32数据接收超时（30秒）

WiFiUDP udp;
unsigned long lastDataTime = 0;

void setup() {
  Serial.begin(DEBUG_BAUDRATE);
  delay(1000);
  Serial.println("\n[ESP8266] 通信模块启动");

  // 1. 连接Wi-Fi
  connectWiFi();

  // 2. 初始化UDP
  udp.begin(0);
  Serial.println("UDP初始化完成 | 目标服务器: " + String(targetIPStr) + ":" + String(targetPort));
}

void loop() {
  // === 任务1：处理STM32数据 ===
  processSTM32Data();

  // === 任务2：监控Wi-Fi连接 ===
  checkWiFi();

  delay(10);
}

// ===== 功能函数 =====

/**
 * 连接Wi-Fi网络
 */
void connectWiFi() {
  Serial.print("正在连接Wi-Fi: " + String(ssid));
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n连接成功! IP地址: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n连接失败! 进入重试模式...");
    while (1) {
      delay(1000);
      if (WiFi.status() == WL_CONNECTED) break;
      Serial.print(".");
    }
  }
}

/**
 * 处理STM32数据
 */
void processSTM32Data() {
  static String buffer; // 静态缓冲区处理数据分片

  while (Serial.available() > 0) {
    char c = Serial.read();
    buffer += c;

    // 检测到换行符（STM32数据结束标志）
    if (c == '\n') {
      buffer.trim(); // 去除首尾空白（包括\r）

      if (buffer.length() > 0) {
        // 转发数据到UDP服务器
        udp.beginPacket(targetIPStr, targetPort);
        udp.write(buffer.c_str());
        udp.endPacket();
        
        Serial.println("[数据转发] " + buffer);
        lastDataTime = millis(); // 更新最后接收时间
      }
      buffer = ""; // 清空缓冲区
    }
  }

  // 检测数据超时
  if (millis() - lastDataTime > DATA_TIMEOUT) {
    Serial.println("[警告] 超过30秒未收到STM32数据！");
    lastDataTime = millis();
  }
}

/**
 * 监控Wi-Fi连接状态
 */
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWi-Fi断开，尝试重连...");
    WiFi.reconnect();
    delay(2000);

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("重连成功！");
    } else {
      Serial.println("重连失败！");
    }
  }
}
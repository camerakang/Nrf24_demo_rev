
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include "printf.h"

// nRF24L01引脚配置
#define CE_PIN 2
#define CSN_PIN 3
RF24 radio(CE_PIN, CSN_PIN);

// 接收地址
uint8_t address[][6] = {"1Node", "2Node", "3Node"};

struct PayloadStruct
{
  char message[12]; // 传输的消息
  uint16_t counter; // 计数器
};
PayloadStruct payload;

// 记录上一次接收到数据的时间
unsigned long last_received_time = 0;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // 等待串口准备就绪
  }

  if (!radio.begin())
  {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
    {
    } // 进入无限循环
  }
  radio.setCRCLength(RF24_CRC_8);
  radio.setDataRate(RF24_2MBPS);
  radio.setRetries(2, 5);
  radio.setPALevel(RF24_PA_LOW);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openWritingPipe(address[3]);    // 设置发送地址 (用于ACK)
  radio.openReadingPipe(1, address[1]); // 设置接收地址
  radio.startListening();               // 设置为接收模式

  radio.printDetails();
  radio.setChannel(100);

  payload.counter = 0;                                 // 初始化计数器
  memcpy(payload.message, "RX World 0", 14);             // 设置接收消息
  radio.writeAckPayload(1, &payload, sizeof(payload)); // 预载入ACK负载

  // 初始化上一次接收时间
  last_received_time = millis();
}

void loop()
{
  uint8_t pipe;
  if (radio.available(&pipe))
  {                                                             // 检查是否有数据到达
    unsigned long current_time = millis();                      // 当前时间
    unsigned long interval = current_time - last_received_time; // 计算间隔时间
    last_received_time = current_time;                          // 更新上一次接收时间

    uint8_t bytes = radio.getDynamicPayloadSize(); // 获取负载大小
    // PayloadStruct received;
    uint8_t received[bytes] = {0};
    radio.read(received, bytes); // 读取数据
    Serial.print(F("Received "));
    Serial.print(bytes); // 打印负载大小
    Serial.print(F(" bytes on pipe "));
    Serial.print(pipe); // 打印管道编号
    Serial.print(F(": "));
    // Serial.print(received.message); // 打印收到的消息
    // Serial.print(received.counter); // 打印收到的计数器
    // Serial.print(F(" Sent: "));
    // Serial.print(payload.message); // 打印发送的消息
    // Serial.print(payload.counter); // 打印发送的计数器
    // 打印接收到的数据
    for (int i = 0; i < bytes; i++)
    {
      Serial.print("0x");
      Serial.print((received[i]));
      Serial.print(" ");
    }
    Serial.print(F(" Interval: "));
    Serial.print(interval); // 打印时间间隔
    Serial.println(F(" ms"));
    // payload.counter = received.counter + 1;              // 更新计数器
    payload.counter = received[0]+1;
    radio.writeAckPayload(1, &payload, sizeof(payload)); // 预载入新的ACK负载
  }
}

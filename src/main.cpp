
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include "printf.h"
#include "NRF24_device.h"
#include <iostream>

SPIClass rf24_spi(HSPI);

void TaskReceiveFromSend(void *pvParameters);

// nRF24L01引脚配置
#define CE_PIN 2
#define CSN_PIN 3
// RF24 radio(CE_PIN, CSN_PIN);

// 接收地址
uint8_t address[][6] = {"1Node", "2Node", "3Node"};

// struct PayloadStruct
// {
//   char message[12] = {"PayLoad ack"}; // 传输的消息
//   uint16_t counter = 0;               // 计数器
// };
// PayloadStruct payload;

// 记录上一次接收到数据的时间
unsigned long last_received_time = 0;

void setup()
{
  Serial.begin(115200);
  std::cout << "__cplusplus: " << __cplusplus << std::endl;

  while (!Serial)
  {
    // 等待串口准备就绪
  }
  rf24_spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);
  if (!radio.begin(&rf24_spi, CE_PIN, CSN_PIN))
  {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
    {
    } // 进入无限循环
  }
  radio.setCRCLength(RF24_CRC_8);
  radio.setDataRate(RF24_1MBPS);
  radio.setRetries(2, 5);
  radio.setPALevel(RF24_PA_HIGH);
  radio.enableAckPayload();

  radio.enableDynamicPayloads();
  radio.openWritingPipe(address[3]);    // 设置发送地址 (用于ACK)
  radio.openReadingPipe(1, address[1]); // 设置接收地址

  radio.printDetails();
  radio.setChannel(100);
  radio.flush_tx();
  memcpy(&ackPayload, "first ack", sizeof("first ack"));

  radio.writeAckPayload(1, &payload, sizeof(payload)); // 预载入ACK负载
  radio.startListening();                              // 设置为接收模式
  ackPayload.counter = 0;
  // xTaskCreate(
  //     TaskReceiveFromSend, // 任务函数
  //     "ReceiveFromSend",   // 任务名称
  //     2048,                // 堆栈大小（字节）
  //     NULL,                // 任务参数
  //     1,                   // 任务优先级
  //     NULL                 // 任务句柄
  // );
  // 初始化上一次接收时间
  last_received_time = millis();
}

void loop()
{
  // 调用函数
  ReceivedData receivedData = handleRadioReceive(&ackPayload, sizeof(ackPayload));
  if (receivedData.size > 0)
  {
    memcpy(&ackPayload, "payload ack", sizeof("payload ack"));
    ackPayload.counter = receivedData.data[2];
    radio.writeAckPayload(1, &payload, sizeof(payload)); // 预载入ACK负载
  }

  delay(1);
}
void TaskReceiveFromSend(void *pvParameters)
{
  while (1)
  {
    uint8_t pipe;
    if (radio.available(&pipe))
    { // is there a payload?
      auto payloadsize = radio.getDynamicPayloadSize();
      uint8_t received[payloadsize] = {0};
      radio.read(&received, sizeof(received)); // get incoming payload
      Serial.print("Received ");
      Serial.print(payloadsize);
      Serial.print(" bytes on pipe ");
      Serial.print(pipe);
      Serial.print(": ");
      for (int i = 0; i < payloadsize; i++)
      {
        Serial.print("0x");
        Serial.print((received[i]),HEX);
        Serial.print(" ");
      }

      // transmit response & save result to `report`
      radio.stopListening(); // put in TX mode

      radio.writeFast(&payload, sizeof(payload), false); // load response to TX FIFO
      // radio.txStandBy(150);                       // keep retrying for 150 ms

      radio.startListening(); // put back in RX mode
    }
    vTaskDelay(10);
  }
}
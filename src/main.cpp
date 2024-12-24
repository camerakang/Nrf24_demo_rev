
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include "printf.h"
#include "NRF24_device.h"
#include <iostream>
#include "simple_protocol_impl.h"
#include "simple_protocol_tpl.h"
SimpleProtocolImpl *sprotocol_recv = dynamic_cast<SimpleProtocolImpl *>(new SimpleProtocolTpl<1, 1, true, 32, 0>({0xA5, 0xA5}));

void setup()
{
  Serial.begin(115200);
  std::cout << "__cplusplus: " << __cplusplus << std::endl;

  while (!Serial)
  {
    // 等待串口准备就绪
  }
  rf24_init_recv();
}

void loop()
{
  send_buffer[0] = 0xaa;
  memset(recv_buffer, 0, 32);
  auto len = rf24_recv(recv_buffer, send_buffer, 32);
  auto frms{sprotocol_recv->push_back(recv_buffer, len).parse_all()};
  for (auto &frm : frms)
  {
    if (frm.is_valid())
    {
      if (frm.channel == 1)
      {
        rf24_change_channel(1);
        for (int i = 0; i < sizeof(frm.data); i++)
        {
          Serial.print(frm.data[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      } 
    }
  }
  delay(10);
}
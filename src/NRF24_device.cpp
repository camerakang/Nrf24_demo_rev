#include "NRF24_device.h"

uint8_t recv_buffer[32]{"recv buffer is empty"};
uint8_t send_buffer[32]{"send buffer is empty"};

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);
SPIClass rf24_spi(HSPI);

// an identifying device destination
// Let these addresses be used for the pair
uint8_t address[][6] = {"1Node", "2Node"};
uint8_t receive_address[][6]={"3Node","4Node","5Node","6Node"};

void rf24_init_send()
{

    rf24_spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

    // initialize the transceiver on the SPI bus
    if (!radio.begin(&rf24_spi, CE_PIN, CSN_PIN))
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
        } // hold in infinite loop
    }

    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

    // to use ACK payloads, we need to enable dynamic payload lengths (for all nodes)
    radio.enableDynamicPayloads(); // ACK payloads are dynamically sized

    // Acknowledgement packets have no payloads by default. We need to enable
    // this feature for all nodes (TX & RX) to use ACK payloads.
    radio.enableAckPayload();

    // set the TX address of the RX node into the TX pipe
    radio.openWritingPipe(address[0]); // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, receive_address[1]); // using pipe 1

    radio.stopListening(); // this also discards any unused ACK payloads
    // For debugging info
    // printf_begin(); // needed only once for printing details
    // radio.printDetails();       // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data
}
void rf24_init_recv()
{
    rf24_spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

    // initialize the transceiver on the SPI bus
    if (!radio.begin(&rf24_spi, CE_PIN, CSN_PIN))
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
        } // hold in infinite loop
    }

    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

    // to use ACK payloads, we need to enable dynamic payload lengths (for all nodes)
    radio.enableDynamicPayloads(); // ACK payloads are dynamically sized

    // Acknowledgement packets have no payloads by default. We need to enable
    // this feature for all nodes (TX & RX) to use ACK payloads.
    radio.enableAckPayload();
    radio.setAutoAck(true);
    // set the TX address of the RX node into the TX pipe
    radio.openWritingPipe(address[0]); // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, receive_address[2]); // using pipe 1

    // load the payload for the first received transmission on pipe 0
    radio.writeAckPayload(1, send_buffer, sizeof(send_buffer));
    radio.startListening();
    // For debugging info
    // printf_begin(); // needed only once for printing details
    radio.printDetails(); // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data
}
const uint8_t num_channels = 126; // 0-125 are supported
uint8_t values[num_channels];     // the array to store summary of signal counts per channel
const uint8_t noiseAddress[][2] = {{0x55, 0x55}, {0xAA, 0xAA}, {0xA0, 0xAA}, {0xAB, 0xAA}, {0xAC, 0xAA}, {0xAD, 0xAA}};
const int num_reps = 100;  // number of passes for each scan of the entire spectrum
bool constCarrierMode = 0; // this flag controls example behavior (scan mode is default)

void printHeader()
{
    // Print the hundreds digits
    for (uint8_t i = 0; i < num_channels; ++i)
        Serial.print(i / 100);
    Serial.println();

    // Print the tens digits
    for (uint8_t i = 0; i < num_channels; ++i)
        Serial.print((i % 100) / 10);
    Serial.println();

    // Print the singles digits
    for (uint8_t i = 0; i < num_channels; ++i)
        Serial.print(i % 10);
    Serial.println();

    // Print the header's divider
    for (uint8_t i = 0; i < num_channels; ++i)
        Serial.print(F("~"));
    Serial.println();
}
void rf24_init_scanner()
{
    rf24_spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

    // initialize the transceiver on the SPI bus
    if (!radio.begin(&rf24_spi, CE_PIN, CSN_PIN))
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
        } // hold in infinite loop
    }
    radio.stopConstCarrier(); // in case MCU was reset while radio was emitting carrier wave
    radio.setAutoAck(false);  // Don't acknowledge arbitrary signals
    radio.disableCRC();       // Accept any signal we find
    radio.setAddressWidth(2); // A reverse engineering tactic (not typically recommended)
    for (uint8_t i = 0; i < 6; ++i)
    {
        radio.openReadingPipe(i, noiseAddress[i]);
    }

    // set the data rate
    Serial.print(F("Select your Data Rate. "));
    Serial.print(F("Enter '1' for 1 Mbps, '2' for 2 Mbps, '3' for 250 kbps. "));
    Serial.println(F("Defaults to 1Mbps."));
    while (!Serial.available())
    {
        // wait for user input
    }
    uint8_t dataRate = Serial.parseInt();
    if (dataRate == 50)
    {
        Serial.println(F("Using 2 Mbps."));
        radio.setDataRate(RF24_2MBPS);
    }
    else if (dataRate == 51)
    {
        Serial.println(F("Using 250 kbps."));
        radio.setDataRate(RF24_250KBPS);
    }
    else
    {
        Serial.println(F("Using 1 Mbps."));
        radio.setDataRate(RF24_1MBPS);
    }
    Serial.println(F("***Enter a channel number to emit a constant carrier wave."));
    Serial.println(F("***Enter a negative number to switch back to scanner mode."));

    // Get into standby mode
    radio.startListening();
    radio.stopListening();
    radio.flush_rx();

    // printf_begin();
    // radio.printPrettyDetails();
    // delay(1000);

    // Print out vertical header
    printHeader();
}
void rf24_sacnner_loop()
{
    /****************************************/
    // Send a number over Serial to begin Constant Carrier Wave output
    // Configure the power amplitude level below
    if (Serial.available())
    {
        int8_t c = Serial.parseInt();
        if (c >= 0)
        {
            c = min(125, max(0, static_cast<int>(c))); // clamp channel to supported range
            constCarrierMode = 1;
            radio.stopListening();
            delay(2);
            Serial.print("\nStarting Carrier Wave Output on channel ");
            Serial.println(c);
            // for non-plus models, startConstCarrier() changes address on pipe 0 and sets address width to 5
            radio.startConstCarrier(RF24_PA_LOW, c);
        }
        else
        {
            constCarrierMode = 0;
            radio.stopConstCarrier();
            radio.setAddressWidth(2);                  // reset address width
            radio.openReadingPipe(0, noiseAddress[0]); // ensure address is looking for noise
            Serial.println("\nStopping Carrier Wave Output");
            printHeader();
        }

        // discard any CR and LF sent
        while (Serial.peek() != -1)
        {
            if (Serial.peek() == '\r' || Serial.peek() == '\n')
            {
                Serial.read();
            }
            else
            {          // got a charater that isn't a line feed
                break; // handle it on next loop() iteration
            }
        }
    }

    /****************************************/

    if (constCarrierMode == 0)
    {
        // Clear measurement values
        memset(values, 0, sizeof(values));

        // Scan all channels num_reps times
        int rep_counter = num_reps;
        while (rep_counter--)
        {
            int i = num_channels;
            while (i--)
            {
                // Select this channel
                radio.setChannel(i);

                // Listen for a little
                radio.startListening();
                delayMicroseconds(128);
                bool foundSignal = radio.testRPD();
                radio.stopListening();

                // Did we get a signal?
                if (foundSignal || radio.testRPD() || radio.available())
                {
                    ++values[i];
                    radio.flush_rx(); // discard packets of noise
                }
            }
        }

        // Print out channel measurements, clamped to a single hex digit
        for (int i = 0; i < num_channels; ++i)
        {
            if (values[i])
                Serial.print(min((uint8_t)0xf, values[i]), HEX);
            else
                Serial.print(F("-"));
        }
        Serial.println();

    } // if constCarrierMode == 0
    else
    {
        // show some output to prove that the program isn't bricked
        Serial.print(F("."));
        delay(1000); // delay a second to keep output readable
    }
}
size_t rf24_send(uint8_t *send_buffer, int send_len, uint8_t *recv_buffer)
{
    radio.setAutoAck(true);
    unsigned long start_timer = micros(); // start the timer
    // Serial.print(F("Sending data: "));
    // for (size_t i = 0; i < send_len; i++)
    // {
    //     Serial.print(send_buffer[i], HEX);
    //     Serial.print(" ");
    // }
    // Serial.println();

    bool report = radio.writeFast(send_buffer, send_len); // transmit & save the report
    unsigned long end_timer = micros();                   // end the timer

    if (report)
    {
        // Serial.print(F("Transmission successful! ")); // payload was delivered
        // Serial.print(F("Time to transmit = "));
        // Serial.print(end_timer - start_timer); // print the timer result
        // Serial.print(F(" us. Sent: "));

        uint8_t pipe;
        if (radio.available(&pipe))
        { // is there an ACK payload? grab the pipe number that received it
            size_t bytes = radio.getDynamicPayloadSize();
            radio.read(recv_buffer, bytes); // get incoming ACK payload
            // Serial.print(F(" Received "));
            // Serial.print(bytes); // print incoming payload size
            // Serial.print(F(" bytes on pipe "));
            // Serial.print(pipe); // print pipe number that received the ACK
            // Serial.print(F(": "));
            // for (uint8_t i = 0; i < radio.getDynamicPayloadSize(); i++)
            // {
            //     Serial.print(recv_buffer[i], HEX);
            //     Serial.print(" ");
            // }
            // Serial.println();
            return bytes;
        }
        else
        {
            Serial.println(F(" Received: an empty ACK packet")); // empty ACK packet received
            return 0;
        }
    }
    else
    {
        Serial.println(F("Transmission failed or timed out")); // payload was not delivered
        return 0;
    }
}
void rf24_send_only(uint8_t *send_buffer, int send_len)
{
    radio.stopListening();
    radio.setAutoAck(false);
    radio.writeFast(send_buffer, send_len); // transmit & save the report
}

size_t rf24_recv(uint8_t *recv_buffer, uint8_t *send_buffer, uint8_t send_len)
{
    radio.startListening();
    uint8_t pipe;
    if (radio.available(&pipe))
    {                                                  // is there a payload? get the pipe number that received it
        uint8_t bytes = radio.getDynamicPayloadSize(); // get the size of the payload
        radio.read(recv_buffer, bytes);                // get incoming payload
        Serial.print(F("Received "));
        Serial.print(bytes); // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.print(F(": "));
        // for (uint8_t i = 0; i < bytes; i++)
        // {
        //     Serial.print(recv_buffer[i], HEX);
        // }
        // Serial.println();
        // Serial.print("Sending ACK payload...");
        // for (uint8_t i = 0; i < bytes; i++)
        // {
        //     Serial.print(send_buffer[i], HEX);
        // }
        // Serial.println();
        radio.writeAckPayload(1, send_buffer, send_len);
        return bytes;
    }
    return 0;
}

size_t rf24_recv_only(uint8_t *recv_buffer)
{
    radio.startListening();
    uint8_t pipe;
    if (radio.available(&pipe))
    {                                                  // is there a payload? get the pipe number that received it
        uint8_t bytes = radio.getDynamicPayloadSize(); // get the size of the payload
        radio.read(recv_buffer, bytes);                // get incoming payload
        // Serial.print(F("Received "));
        // Serial.print(bytes); // print the size of the payload
        // Serial.print(F(" bytes on pipe "));
        // Serial.print(pipe); // print the pipe number
        // // Serial.print(F(": "));
        // for (uint8_t i = 0; i < bytes; i++)
        // {
        //     Serial.print(recv_buffer[i], HEX);
        // }
        // Serial.println();
        // Serial.print("Sending ACK payload...");
        // for (uint8_t i = 0; i < bytes; i++)
        // {
        //     Serial.print(send_buffer[i], HEX);
        // }
        // Serial.println();
        return bytes;
    }
    return 0;
}

void rf24_change_channel(uint8_t channel)
{
    auto cha = radio.getChannel();
    Serial.print("Current channel: ");
    Serial.println(cha);
    bool goodSignal = radio.testRPD();
    Serial.println(goodSignal ? "Strong signal > -64dBm" : "Weak signal < -64dBm");
    radio.stopListening();
    // radio.setChannel(channel);
}
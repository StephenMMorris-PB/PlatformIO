#include "AzureIotHub.h"
#include "AZ3166WiFi.h"
#include "config.h"
#include "utility.h"
#include "iothub_client_sample_mqtt.h"
#include "Telemetry.h"
//smm-added GPS header
#include "Adafruit_GPS.h"

// smm: 3/10/2020, 2:10PM status:
//                        1. Solved: Adafruit GPS errors solved (with Adafruit_GPS.h added above)
//                        2. Next step: Add 2nd UART

// smm: adding 2nd UART. . .

UARTClass Serial1(UART_1);

static bool hasWifi = false;
int messageCount = 1;

void initWifi()
{
     Screen.print("Geo Device1\r\n \r\nConnecting...\r\n");

    if (WiFi.begin() == WL_CONNECTED)
    {
        IPAddress ip = WiFi.localIP();
        Screen.print(1, ip.get_address());
        hasWifi = true;
        Screen.print(2, "3/11/20 3:20 pm \r\n");
        Screen.print(3, "Goal: show L/L\r\n");
    }
    else
    {
        Screen.print(1, "No Wi-Fi\r\n ");
    }
}

void setup()
{
    //smm-setting up 2nd UART for GPS data
    Serial1.begin(9600);

    hasWifi = false;
    initWifi();
    if (!hasWifi)
    {
        LogInfo("Please make sure the wifi connected!");
        return;
    }
    
    // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
    // We collect data about the features you use, how often you use them, and how you use them.
    send_telemetry_data("", "HappyPathSetup", "");

    Serial.begin(115200);
    sensorInit();
    iothubInit();
}

void loop()
{
    //char messagePayload[MESSAGE_MAX_LEN];
    //bool temperatureAlert = readMessage(messageCount, messagePayload);
    //iothubSendMessage((const unsigned char *)messagePayload, temperatureAlert);
    //iothubLoop();
    // smm: Setting up GPS data XMIT -> MXChip -> Serial Monitor
    // smm: Read a byte from 2nd serial port
    int byte = Serial1.read();
    //smm: Send the byte to the USB serial port
    Serial.write(byte);

    delay(10);
}

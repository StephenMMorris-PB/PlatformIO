#include "AzureIotHub.h"
#include "AZ3166WiFi.h"
#include "config.h"
#include "utility.h"
#include "iothub_client_sample_mqtt.h"
#include "Telemetry.h"

static bool hasWifi = false;
int messageCount = 1;

void initWifi()
{
     Screen.print("geo-Device1\r\n \r\nConnecting...\r\n");

    if (WiFi.begin() == WL_CONNECTED)
    {
        IPAddress ip = WiFi.localIP();

        hasWifi = true;

        //Screen.print(1, ip.get_address());
        Screen.print(1, "3/21/20 9:54am \r\n"); //time
        //Screen.print(2, "T,H,L,L->SqlSvr\r\n");
        Screen.print(2, "L,L->SerMon\r\n");
        Screen.print(3, "ConfirmDataQlty\r\n");
    }
    else
    {
        Screen.print(3, "No Wi-Fi\r\n ");
    }
}

void setup()
{
    //smm-setting up 2nd UART for GPS data
    //GpsSerial.begin(9600);  //GPS UART baud rate
    GPSsetup(); //

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
    // Data: Temp, Hum, Lat, Lon -> IoT Hub
    char messagePayload[MESSAGE_MAX_LEN];
    bool temperatureAlert = readMessage(messageCount, messagePayload);
    iothubSendMessage((const unsigned char *)messagePayload, temperatureAlert);
    iothubLoop();  
                   
    
    // Data: NMEA sentences (including raw Lat, Lon) -> Serial Monitor (laptop)
    //displayNMEAsentencesOnMonitor(); //smm: uncomment this for display of complete NMEA sentences, $GPRMC, $GPGGA (Lat/Lon contained in these)
}
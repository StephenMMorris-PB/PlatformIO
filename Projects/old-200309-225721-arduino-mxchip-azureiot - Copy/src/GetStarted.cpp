#include "AzureIotHub.h"
#include "AZ3166WiFi.h"
#include "config.h"
#include "utility.h"
#include "iothub_client_sample_mqtt.h"
#include "Telemetry.h"

int messageCount = 1;

void setup()
{
    Screen.print(1, "3/20/20   2:38pm\r\n");
    Screen.print(2, "T,H,L,L->AzSqlSvr\r\n");
    Screen.print(3, "Tbl=temhumlatlon\r\n");
    
    WifiInit();
    gpsInit();


    Serial.begin(115200);
    sensorInit();
    iothubInit();
}

void loop()
{
    //smm-3/13/2020-re-activating IoT hub msg XFER functions
    char messagePayload[MESSAGE_MAX_LEN];
    bool temperatureAlert = readGeoMessage(messageCount, messagePayload);
    iothubSendMessage((const unsigned char *)messagePayload, temperatureAlert);
    iothubLoop();  //smm-03/15/2020, 7:15am - added back after accidental deletion.
                   // this deletion may be reason data not sent to IoT Hub.
    delay(10);
}

#include "AzureIotHub.h"
#include "AZ3166WiFi.h"
#include "config.h"
#include "utility.h"
#include "iothub_client_sample_mqtt.h"
#include "Telemetry.h"

// ***** START - GPS Code - per toptechboy.com *****
#include "Adafruit_GPS.h"   //smm

UARTClass GpsSerial(UART_1);  // create 2nd UART for GPS->MXChip Lat/Long XFER
Adafruit_GPS GPS(&GpsSerial); // create GPS object, using 2nd UART (UART_1, w/ name GpsSerial)

String NMEA1;  // Variable to hold our first NMEA sentence
String NMEA2;  // Variable to hold our second NMEA sentence


char c;        // Read the characters from the GPS module
// ***** END - GPS Code - per toptechboy.com *****

// to format NMEA Latitude, Longitude
//smm-3/16/20 - changing double -> float, 5 places for deg, degWhole, degDec...
float deg;          // Will hold position data in simple degree format
float degWhole;     // Variable for the whole part of position 
float degDec;       // Variable for the decimal part of degree

static bool hasWifi = false;
int messageCount = 1;


void initWifi()
{
     Screen.print("geo-Device1\r\n \r\nConnecting...\r\n");

    if (WiFi.begin() == WL_CONNECTED)
    {
        IPAddress ip = WiFi.localIP();
        //Screen.print(1, ip.get_address());
        Screen.print(1, "3/19/20 3:07pm \r\n");
        hasWifi = true;
        //Screen.print(2, "Lat,Lon->Ser Mon\r\n");
        //Screen.print(3, "NMEA,Lat,Lon\r\n");
        //Screen.print(3, "Tem,Hud->IoT Hub\r\n");
        Screen.print(3, "Tm,Hu->AzSqlSvr\r\n");
    }
    else
    {
        Screen.print(1, "No Wi-Fi\r\n ");
    }
}


void GPSsetup()  
{
  GPS.sendCommand("$PGCMD,33,0*6D");                // Turn Off GPS Antenna Update
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);     // Tell GPS we want only $GPRMC and $GPGGA NMEA sentences
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);        // 1 Hz update rate
  delay(1000);                                      //Pause
}

void clearGPS() 
{  //Since between GPS reads, we still have data streaming in, we need to clear the old data by reading a few sentences, and discarding these
    while(!GPS.newNMEAreceived()) 
    {
        c=GPS.read();
    }
    GPS.parse(GPS.lastNMEA());
    while(!GPS.newNMEAreceived()) {
    c=GPS.read();
    }
    GPS.parse(GPS.lastNMEA());
}

void readGPS()          //This function will read and remember two NMEA sentences from GPS
{  
    clearGPS();         //Serial port probably has old or corrupt data, so begin by clearing it all out
    while(!GPS.newNMEAreceived()) 
    {   //Keep reading characters in this loop until a good NMEA sentence is received
        c=GPS.read(); //read a character from the GPS
    }
    GPS.parse(GPS.lastNMEA());  //Once get a good NMEA, parse it
    
    NMEA1=GPS.lastNMEA();       //Once parsed, save NMEA sentence into NMEA1
    
    while(!GPS.newNMEAreceived()) 
    {   //Get 2nd NMEA sentence, should be different type than the first one read above.
        c=GPS.read();
    }
    GPS.parse(GPS.lastNMEA());
    
    NMEA2=GPS.lastNMEA();
  
    //smm: 3/15/2020: uncommented 2 NMEA prints below to see full NMEA sentences in serial monitor.
    //Serial.println(NMEA1);
    //Serial.println(NMEA2);
    Serial.println("");


    // ************************************************************************************
    //smm-NMEA format to Latitude, Longitude

    degWhole = float(int(GPS.longitude/100)); //gives me the whole degree part of Longitude
    degDec = (GPS.longitude - degWhole*100)/60; //give me fractional part of longitude
    deg = degWhole + degDec; //Gives complete correct decimal form of Longitude degrees
    if (GPS.lon =='W') 
    {  //If you are in Western Hemisphere, longitude degrees should be negative
        deg= (-1)*deg;
    }

    Serial.println(deg);

    degWhole = float(int(GPS.latitude/100)); //gives me the whole degree part of latitude
    degDec = (GPS.latitude - degWhole*100)/60; //give me fractional part of latitude
    deg = degWhole + degDec; //Gives complete correct decimal form of latitude degrees
    if (GPS.lat =='S') 
    {  //If you are in Southern hemisphere latitude should be negative
        deg= (-1)*deg;
    }
    
    Serial.println(deg);
    Serial.println("");
    
    // ************************************************************************************
}



void setup()
{
    //smm-setting up 2nd UART for GPS data
    GpsSerial.begin(9600);  //GPS UART baud rate
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
    //smm-3/13/2020-re-activating IoT hub msg XFER functions
    char messagePayload[MESSAGE_MAX_LEN];
    bool temperatureAlert = readMessage(messageCount, messagePayload);
    iothubSendMessage((const unsigned char *)messagePayload, temperatureAlert);
    iothubLoop();  //smm-03/15/2020, 7:15am - added back after accidental deletion.
                   // this deletion may be reason data not sent to IoT Hub.
    // readGPS();  // Let's see clean NMEA's in MXChip display!

    delay(10);
}

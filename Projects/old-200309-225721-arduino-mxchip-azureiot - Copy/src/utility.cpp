// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "HTS221Sensor.h"
#include "AzureIotHub.h"
#include "Arduino.h"
#include <ArduinoJson.h>
#include "config.h"
#include "RGB_LED.h"
#include "AZ3166WiFi.h"
#include "utility.h"

#include "Adafruit_GPS.h"   //smm



#define RGB_LED_BRIGHTNESS 32

DevI2C *i2c;
HTS221Sensor *sensor;
static RGB_LED rgbLed;
static int interval = INTERVAL;


UARTClass GpsSerial(UART_1);  // create 2nd UART for GPS->MXChip Lat/Long XFER
Adafruit_GPS GPS(&GpsSerial); // create GPS object, using 2nd UART (UART_1, w/ name GpsSerial)

String NMEA1;  // Variable to hold our first NMEA sentence
String NMEA2;  // Variable to hold our second NMEA sentence

static bool hasWifi = false;

char c;        // Read the characters from the GPS module


float latitude;
float longitude;
float degWhole;     // Variable for the whole part of position 
float degDec;       // Variable for the decimal part of degree





int getInterval()
{
    return interval;
}

void blinkLED()
{
    rgbLed.turnOff();
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    delay(500);
    rgbLed.turnOff();
}

void blinkSendConfirmation()
{
    rgbLed.turnOff();
    rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
    delay(500);
    rgbLed.turnOff();
}

void parseTwinMessage(const char *message)
{
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(message);
    if (!root.success())
    {
        LogError("parse %s failed", message);
        return;
    }

    if (root["desired"]["interval"].success())
    {
        interval = root["desired"]["interval"];
    }
    else if (root.containsKey("interval"))
    {
        interval = root["interval"];
    }
}


void WifiInit()
{

    //hasWifi = false;  <---herey - not sure if this needs to be moved somewhere else

    // Screen.print(DEVICE_ID);
    Screen.print("geo-Device1\r\n \r\nConnecting...\r\n");

    if (WiFi.begin() == WL_CONNECTED)
    {
        IPAddress ip = WiFi.localIP();
        //Screen.print(1, ip.get_address());
        //Screen.print(1, "3/20/20 9:00Am \r\n");
        hasWifi = true;
    }
    else
    {
        Screen.print(1, "No Wi-Fi\r\n ");
    }
    if (!hasWifi)
    {
        LogInfo("Please make sure the wifi connected!");
        return;  //smm: this return could require additional return - nested level change
    }
    //return; //smm-herey - added this return--may want to confirm/remove this.
}




void sensorInit()
{
    i2c = new DevI2C(D14, D15);
    sensor = new HTS221Sensor(*i2c);
    sensor->init(NULL);
}

float readTemperature()
{
    sensor->reset();

    float temperature = 0;
    sensor->getTemperature(&temperature);

    return temperature;
}

float readHumidity()
{
    sensor->reset();

    float humidity = 0;
    sensor->getHumidity(&humidity);

    return humidity;
}

void GPSsetup()  
{
  GPS.sendCommand("$PGCMD,33,0*6D");                // Turn Off GPS Antenna Update
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);     // Tell GPS we want only $GPRMC and $GPGGA NMEA sentences
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);        // 1 Hz update rate
  delay(1000);                                      //Pause
}

void gpsInit()
{
     //smm-setting up 2nd UART for GPS data
    GpsSerial.begin(9600);  //GPS UART baud rate
    GPSsetup(); //
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

// <LatLonPayload>smm: 3/19/2020:
bool readGeoMessage(int messageId, char *payload)
{
    float temperature = readTemperature();
    float humidity = readHumidity();
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["deviceId"] = DEVICE_ID;
    root["messageId"] = messageId;
    
    bool temperatureAlert = false;

    root["latitude"] = latitude;
    root["longitude"] = longitude;

    //readGPS();

// **********************************readGPS-start**********************************************
// *********************************************************************************************

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
    //Serial.println("");

    //smm-NMEA format to Latitude, Longitude

    degWhole = float(int(GPS.longitude/100)); //gives me the whole degree part of Longitude
    degDec = (GPS.longitude - degWhole*100)/60; //give me fractional part of longitude
    longitude = degWhole + degDec; //Gives complete correct decimal form of Longitude degrees
    if (GPS.lon =='W') 
    {  //If you are in Western Hemisphere, longitude degrees should be negative
        longitude= (-1)*longitude;
    }

    //Serial.println(longitude);

    degWhole = float(int(GPS.latitude/100)); //gives me the whole degree part of latitude
    degDec = (GPS.latitude - degWhole*100)/60; //give me fractional part of latitude
    latitude = degWhole + degDec; //Gives complete correct decimal form of latitude degrees
    if (GPS.lat =='S') 
    {  //If you are in Southern hemisphere latitude should be negative
        latitude= (-1)*latitude;
    }
    
    //Serial.println(latitude);
    //Serial.println("");

// *****************************readGPS-end*****************************************************
// *********************************************************************************************


    if(temperature != temperature)
    {
        root["temperature"] = NULL;
    }
    else
    {
        root["temperature"] = temperature;
        if(temperature > TEMPERATURE_ALERT)
        {
            temperatureAlert = true;
        }
    }

    if(humidity != humidity)
    {
        root["humidity"] = NULL;
    }
    else
    {
        root["humidity"] = humidity;
    }

    if(latitude != latitude)
    {
        root["latitude"] = NULL;
    }
    else
    {
        root["latitude"] = latitude;
    }


    if(longitude != longitude)
    {
        root["longitude"] = NULL;
    }
    else
    {
        root["longitude"] = longitude;
    }
   
    root.printTo(payload, MESSAGE_MAX_LEN);
    return temperatureAlert;
}
















/*
bool readMessage(int messageId, char *payload)
{
    float temperature = readTemperature();
    float humidity = readHumidity();
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["deviceId"] = DEVICE_ID;
    root["messageId"] = messageId;
    
    bool temperatureAlert = false;

    if(temperature != temperature)
    {
        root["temperature"] = NULL;
    }
    else
    {
        root["temperature"] = temperature;
        if(temperature > TEMPERATURE_ALERT)
        {
            temperatureAlert = true;
        }
    }

    if(humidity != humidity)
    {
        root["humidity"] = NULL;
    }
    else
    {
        root["humidity"] = humidity;
    }
    root.printTo(payload, MESSAGE_MAX_LEN);
    return temperatureAlert;
}
*/
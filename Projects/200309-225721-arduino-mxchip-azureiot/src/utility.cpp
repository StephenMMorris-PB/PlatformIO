// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "HTS221Sensor.h"
#include "AzureIotHub.h"
#include "Arduino.h"
#include <ArduinoJson.h>
#include "config.h"
#include "RGB_LED.h"

#include "Adafruit_GPS.h"  
UARTClass GpsSerial(UART_1);  // 2nd UART for GPS->MXChip Lat/Long XFER
Adafruit_GPS GPS(&GpsSerial); // GPS object, using 2nd UART (UART_1, w/ name GpsSerial)
char c;        // Read the characters from the GPS module

#define RGB_LED_BRIGHTNESS 32

DevI2C *i2c;
HTS221Sensor *sensor;
static RGB_LED rgbLed;
static int interval = INTERVAL;
//smm: Optional serial monitor display of NMEA1: "GPGGA" & NMEA2:"GPRMC"
void displayNMEAsentencesOnMonitor()
{
    // NMEA sentences are captured in a stream, so NMEA1 and NMEA2 represent, depending on stream-timing, either GPGGA or GPRMC
    String NMEA1;  // Variable to hold our first NMEA sentence
    String NMEA2;  // Variable to hold our second NMEA sentence

    //clearGPS();         //Serial port probably has old or corrupt data, so begin by clearing it all out
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
  
    //Serial.println("NMEA1: GPGGA or GPRMC...");
    Serial.println(NMEA1);
    //Serial.println("NMEA2: GPRMC or GPGGA...");
    Serial.println(NMEA2);
    Serial.println("");


}

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
    GpsSerial.begin(9600);  //GPS UART baud rate
    GPS.sendCommand("$PGCMD,33,0*6D");                // Turn Off Adafruit GPS Antenna Update
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);     // Tell GPS we want only $GPRMC and $GPGGA NMEA sentences
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);        // 1 Hz update rate
    delay(2000);                                      //Pause
}

void clearGPS() 
{   //Since between GPS reads, we still have data streaming in, we need to clear the old data by reading a few sentences, and discarding these
    //smm-3/21/2020: added third read sequence to ensure that will get clean data
    while(!GPS.newNMEAreceived()) 
    {
        c=GPS.read();
    }
    GPS.parse(GPS.lastNMEA());
    while(!GPS.newNMEAreceived()) {
    c=GPS.read();
    }
    GPS.parse(GPS.lastNMEA());
    while(!GPS.newNMEAreceived()) {
    c=GPS.read();
    }
    GPS.parse(GPS.lastNMEA());
}

float readLatitude()
{
    float latitude;         // Latitude (degrees)
    float latDegWhole;      // Latitude - Variable for the whole part of position 
    float latDegDec;        // Latitude - Variable for the decimal part of d

    clearGPS();

    //Latitude (degrees) 
    latDegWhole = float(int(GPS.latitude/100)); //gives me the whole degree part of latitude
    latDegDec = (GPS.latitude - latDegWhole*100)/60; //give me fractional part of latitude
    latitude = latDegWhole + latDegDec; //Gives complete correct decimal form of latitude degrees
    if (GPS.lat =='S') 
    {  //If in Southern hemisphere latitude should be negative
        latitude= (-1)*latitude;
    }
    //Serial.println(latitude);
     return latitude;

}

float readLongitude()
{
    float longitude;        // Longitude (degrees)
    float lonDegWhole;      // Longitude - Variable for the whole part of position 
    float lonDegDec;        // Longitude - Variable for the decimal part of degree

    clearGPS();

    // Longitude (degrees)
    lonDegWhole = float(int(GPS.longitude/100)); //gives me the whole degree part of Longitude
    lonDegDec = (GPS.longitude - lonDegWhole*100)/60; //give me fractional part of longitude
    longitude = lonDegWhole + lonDegDec; //Gives complete correct decimal form of Longitude degrees
    if (GPS.lon =='W') 
    {  //If in Western Hemisphere, longitude degrees should be negative
        longitude= (-1)*longitude;
    }

    //Serial.println(longitude);
     return longitude;
}


bool readMessage(int messageId, char *payload)
{
    float temperature = readTemperature();
    float humidity = readHumidity();

    float latitude = readLatitude();
    float longitude = readLongitude();

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
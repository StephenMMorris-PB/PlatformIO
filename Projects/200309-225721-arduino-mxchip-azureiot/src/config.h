// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

// Interval time(ms) for sending message to IoT Hub
#define INTERVAL 2000

#define MESSAGE_MAX_LEN 512 //smm: changed from 256, when only (original 2 values) temperature & humidity were sent in JSON object.
// All 4 values are of same type: 'float' - temperature, humidity, latitude, and longitude, so doubled value.

//smm-3/19/2020: updated definition of DEVICE_ID from default: "AZ3166"
#define DEVICE_ID "geo-Device1"

#define TEMPERATURE_ALERT 30
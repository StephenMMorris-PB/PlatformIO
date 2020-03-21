// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

// Interval time(ms) for sending message to IoT Hub
#define INTERVAL 4000
//smm default value for INTERVAL is 2000.. increasing to 4000 to see if this fix message-send suspension issue.

//#define MESSAGE_MAX_LEN 256
#define MESSAGE_MAX_LEN 512 // <LatLonPayload>smm: 3/19/2020: increased from 256 to 512
//                             ... because added 2 more floats: latitude, longitude
//                             ... to the original 2 floats: temperature & humidity

//smm-3/19/2020: updated definition of DEVICE_ID from default: "AZ3166", to "geo-Device1"
//smm-3/19/2020, 5:34PM: changed to "Port-1433:Off!" for temporary test of turning off this Public IP port on my router
// .. .Eric says I don't need this port on to access SS database, so I'm turning off, and running telementry data to db to confirm.
//#define DEVICE_ID "TstDevID-2DB"
#define DEVICE_ID "geo-Device1"
#define TEMPERATURE_ALERT 30
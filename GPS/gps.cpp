#include "gps.h"
#include "Config.h"
#include "TinyGPSPlus/TinyGPS++.h"
#include <stdio.h>


static TinyGPSPlus gps;

void GPS_Init() {
    //TODO
//    GPS_SERIAL.begin(9600);
    printf("GPS: TinyGPS++ library v. %s\n", TinyGPSPlus::libraryVersion());
}

void GPS_Update() {
    //TODO
//    while (GPS_SERIAL.available() > 0)
//    {
//        char c = GPS_SERIAL.read();
//        gps.encode(c);
//    }
}

bool GPS_GetInfo(HAL::GPS_Info_t *info) {
    static double lon = CONFIG_GPS_LONGITUDE_DEFAULT;
    static double lat = CONFIG_GPS_LATITUDE_DEFAULT;
    memset(info, 0, sizeof(HAL::GPS_Info_t));
    //TODO
//    info->isVaild = gps.location.isValid();
//    info->longitude = gps.location.lng();
//    info->latitude = gps.location.lat();
//    info->altitude = gps.altitude.meters();
//    info->speed = gps.speed.kmph();
//    info->course = gps.course.deg();
//
//    info->clock.year = gps.date.year();
//    info->clock.month = gps.date.month();
//    info->clock.day = gps.date.day();
//    info->clock.hour = gps.time.hour();
//    info->clock.minute = gps.time.minute();
//    info->clock.second = gps.time.second();
//    info->satellites = gps.satellites.value();

    lon  =  lon - 0.00003f;
    lat  =  lat - 0.00003f;

    info->clock.year = 2022;

    info->isVaild = true;
    info->satellites = 10;
    info->longitude = lon;
    info->latitude = lat;

    return info->isVaild;
}

bool GPS_LocationIsValid() {
    //TODO
//    return gps.location.isValid();
    return true;
}

//returns distance in meters between two positions
double GPS_GetDistanceOffset(HAL::GPS_Info_t *info, double preLong, double preLat) {
    //TODO
//    return gps.distanceBetween(info->latitude, info->longitude, preLat, preLong);
    return 0;
}

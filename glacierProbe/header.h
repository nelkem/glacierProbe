#ifndef HEADER_H
#define HEADER_H

/**********************************************************************************************************
  -----Includes-----
**********************************************************************************************************/
//libraries
#include <WaspSensorAgrXtr.h>
//user headers
#include <my4G.h>				    //	Custom 4G class that inherits from Wasp4G but adds a few specific functions

keyvalue currData[] = {keyvalue("temperature"),
                       keyvalue("humidity"),
                       keyvalue("pressure"),
                       keyvalue("sonic"),
                       keyvalue("wetness"),
                       keyvalue("solar"),
                       keyvalue("seconds")};
                       
#define KV_TEMPERATURE    0
#define KV_HUMIDITY       1
#define KV_PRESSURE       2
#define KV_SONIC          3
#define KV_WETNESS        4
#define KV_SOLAR          5
#define KV_SECONDS        6

#define DATA_INTERVAL     30
                       
#include "sensors.h"				  //	Custom sensor functions that can be enabled / disabled based on what is connected
#include "datalogging.h"

#define FTP_SERVER        "77.56.53.236"
#define FTP_USER          "Field"
#define FTP_PASS          "EngGeol2018"
#define FTP_PORT          21
#define FTP_FILE          "/FTP/Kitzsteinhorn/"

uint16_t dataInterval = DATA_INTERVAL;

//  frequency to send file to FTP and start new file
#define NEVER             0
#define CONSTANT          1
#define HOURLY            2
#define DAILY             3
#define WEEKLY            4
#define MONTHLY           5

//  file format
#define DAY_MONTH_YEAR    0
#define YEAR_MONTH_DAY    1
#define SINGLE_FILE       2



#endif




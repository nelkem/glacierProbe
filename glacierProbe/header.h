#ifndef HEADER_H
#define HEADER_H

/**********************************************************************************************************
  -----Includes-----
**********************************************************************************************************/
//libraries
#include <WaspSensorAgrXtr.h>
//user headers
#include <my4G.h>				    //	Custom 4G class that inherits from Wasp4G but adds a few specific functions

#define GLACIERPROBE_DEBUG           1            //  1 - print out debugging information
                                                  //  0 - off

#define _BME      1         //  set to 1 if BME sensor is attached
#define _SONIC    1         //  set to 1 if SONIC sensor is attached
#define _PHYTOS   1         //  set to 1 if PHYTOS sensor is attached
#define _SOLAR    1         //  set to 1 if SOLAR sensor is attached

//Array that stores keyvalue objects representing the current key and value of each measurement
keyvalue currData[] = {keyvalue("temperature"),
                       keyvalue("humidity"),
                       keyvalue("pressure"),
                       keyvalue("sonic"),
                       keyvalue("wetness"),
                       keyvalue("solar"),
                       keyvalue("seconds")};    //  timestamp

//  define the indices in currData where each of these is stored.
#define KV_TEMPERATURE    0
#define KV_HUMIDITY       1
#define KV_PRESSURE       2
#define KV_SONIC          3
#define KV_WETNESS        4
#define KV_SOLAR          5
#define KV_SECONDS        6

#define DATA_INTERVAL     120                    //  in seconds

#define FTP_SERVER        "77.56.53.236"          //  IP or url of FTP server
#define FTP_USER          "Field"                 //  username for FTP server access
#define FTP_PASS          "EngGeol2018"           //  password for FTP server access
#define FTP_PORT          21                      //  port for FTP server access
#define FTP_DIR           "/FTP/Kitzsteinhorn/"   //  directory to store file in FTP server

//  frequency to send file to FTP and start new file
#define FTP_UPLOAD_NEVER             0                       //  never communicate with the FTP server
#define FTP_UPLOAD_DAILY             1                       //  send data every day representing all measurements of that day


#define FTP_UPLOAD_RATE              FTP_UPLOAD_DAILY        // rate that the mote uploads data to the FTP server

//  file format
#define DAY_MONTH_YEAR    0                       //  filename = "DD-MM-YY"
#define YEAR_MONTH_DAY    1                       //  filename = "YY-MM-DD"
#define SINGLE_FILE       2                       //  just named after first day

#define SD_FILEFORMAT     YEAR_MONTH_DAY          //  order of date for the file name


char SD_filename [16];                            //  the name of the file stored on the SD
char FTP_filename [40];                           //  the name and directory of the file stored on the FTP server


                       
#include "sensors.h"				  //	Custom sensor functions that can be enabled / disabled based on what is connected
#include "datalogging.h"


#endif




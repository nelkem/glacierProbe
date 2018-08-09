#ifndef HEADER_H
#define HEADER_H

/**********************************************************************************************************
  -----Includes-----
**********************************************************************************************************/
//libraries
#include <WaspSensorAgrXtr.h>
//user headers
#include <my4G.h>				    //	Custom 4G class that inherits from Wasp4G but adds a few specific functions
#include <DS2.h>

#define GLACIERPROBE_DEBUG           1            //  1 - print out debugging information
                                                  //  0 - off

#define _BME      1         //  set to 1 if BME sensor is attached
#define _SONIC    1         //  set to 1 if SONIC sensor is attached
#define _PHYTOS   1         //  set to 1 if PHYTOS sensor is attached
#define _SOLAR    1         //  set to 1 if SOLAR sensor is attached
#define _DS2      1         //  set to 1 if DS-2 sensor is attached

//Array that stores keyvalue objects representing the current key and value of each measurement
keyvalue currData[] = {keyvalue("temperature"),   //  BME
                       keyvalue("humidity"),      //  BME
                       keyvalue("pressure"),      //  BME
                       keyvalue("sonic"),         //  SONIC
                       keyvalue("wetness"),       //  PHYTOS
                       keyvalue("solar"),         //  SOLAR
                       keyvalue("ubar"),          //  DS2
                       keyvalue("vbar"),          //  DS2
                       keyvalue("gust"),          //  DS2
                       keyvalue("wSpeed"),        //  DS2
                       keyvalue("wDirect"),       //  DS2
                       keyvalue("ds2Temp"),       //  DS2
                       keyvalue("seconds")};      //  timestamp

//  define the indices in currData where each of these is stored.
#define KV_TEMPERATURE    0
#define KV_HUMIDITY       1
#define KV_PRESSURE       2
#define KV_SONIC          3
#define KV_WETNESS        4
#define KV_SOLAR          5
#define KV_UBAR           6
#define KV_VBAR           7
#define KV_GUST           8
#define KV_WINDSPEED      9
#define KV_WINDDIRECTION  10
#define KV_DS2TEMPERATURE 11
#define KV_SECONDS        12

#define NUM_KEYVALS       13

#define DATA_INTERVAL     60                     //  in seconds

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

uint8_t lastDate;
char SD_filename [16] = { 0 };                            //  the name of the file stored on the SD
char FTP_filename [40] = {0 };                           //  the name and directory of the file stored on the FTP server
const char UNSENT_FILES_NAME [] PROGMEM = "fList.txt";
extern my4G comms;


uint8_t setFileNames(char*, uint8_t, char*, uint8_t);
uint8_t writeDataSet(keyvalue*,uint8_t, char*);
bool updateTimes(char*, char*);
uint8_t appendUnsentFile();
uint8_t checkUnsentFiles();
uint8_t markSentFile(char*);
                       
#include "sensors.h"				  //	Custom sensor functions that can be enabled / disabled based on what is connected
#include "datalogging.h"


#endif




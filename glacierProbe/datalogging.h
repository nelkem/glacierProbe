#ifndef DATALOGGING_H
#define DATALOGGING_H

#include "header.h"


/*
setFileNames()
Updates the SD_filename to correspond with the current date, then updates FTP_filename's filename by
appending it after the base directory

returns: 
- 0 if directory is successfully saved
- 1 if directory is not successfully saved

 */

uint8_t setFileNames(char* SD_filename,                    // name of file storing data on SD         
                    uint8_t SD_len,                       //  length of the name of the file
                    char* FTP_filename,                   //  name of file and directory for FTP server
                    uint8_t FTP_length)                   //  length of the FTP server directory
{

//  update the SD filename based on the current date and format settings
  switch(SD_FILEFORMAT){
    case(DAY_MONTH_YEAR):
      sprintf_P(SD_filename, PSTR("%.2u-%.2u-%.2u.csv"), RTC.day,RTC.month,RTC.year);
      break;
    case(YEAR_MONTH_DAY):
      sprintf_P(SD_filename, PSTR("%.2u-%.2u-%.2u.csv"), RTC.year,RTC.month,RTC.day);   
      break;
    case(SINGLE_FILE):
      strcpy(SD_filename, "SENSOR_DATA");
      break;
  }
  
  if(FTP_length > sizeof(FTP_DIR) + SD_len - 1)             //  check for name overflow
  {
    //  append the filename to the directory for the FTP server
    strcpy(FTP_filename, FTP_DIR);
    strcpy(FTP_filename + sizeof(FTP_DIR) - 1, SD_filename);

    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Directory successfully created"));
    #endif
    return 0;
  }
  
  #if GLACIERPROBE_DEBUG == 1
    USB.println(F("not enough space in FTP_filename"));
  #endif
  return 1;
}

/*
writeDataSet()
Formats a large characterarray dataString representing the key value pairs. Keys are separated from their
values with equal signs, keyval pairs are separated from each other with commas, and cyles of measurements are
separated with semicolons.

returns: 
- 0 if dataSet is successfully saved to SD file
- 1 if directory is not successfully saved
- 2 if SD card fails to initialize
- 3 if SD failed to append data to file
- 4 if SD failed to close directory
*/

uint8_t writeDataSet(keyvalue* kvs, uint8_t numPairs, char* filename)
{
  const uint8_t DATASIZE = 128;                 //  max length of the data string

  //  Step 1:
  //  set up the datastring to be written to the file
  char dataString [DATASIZE] = {};
  uint8_t len = 0;
  
  for(int pairInd = 0; pairInd < numPairs; pairInd++)
  {
    //append the key string to the datastring
    uint8_t size = sizeof(kvs[pairInd].key);
    char* k = kvs[pairInd].key;
    uint8_t index = 0;                          //  index of character array 'k'
    
    if(len + size < DATASIZE){                  //  if the key won't overflow the datastring
      while( index < size && k[index] != 0){    //  making sure indices are in bounds and the character is not null
        dataString[len] = k[index];
        len++;
        index++;
      }
    }
    else{
      #if GLACIERPROBE_DEBUG == 1
        USB.println(F("data string overflow."));
      #endif
      return 1;
    }

    dataString[len] = '=';                      // separate the key from the value
    len++;

    //append the val string to the datastring
    size = sizeof(kvs[pairInd].val);
    char* v = kvs[pairInd].val;
    index = 0;
    if(len + size < DATASIZE){
      while( index < size && v[index] != 0){
        dataString[len] = v[index];
        len++;
        index++;
      }
    }
    else{
      #if GLACIERPROBE_DEBUG == 1
        USB.println(F("data string overflow."));
      #endif
      return 1;
    }
    
    if(pairInd < numPairs-1)                  //  if it isn't the last keyvalue, separate with a comma
    {
      dataString[len] = ',';
      len++;
    }
  }
  
  if(len < DATASIZE-1)                        //  if doing so won't overflow the string
  {
  dataString[len] = ';';                      //  add a semicolon to delineate successive measurement cycles
  len++;
  dataString[len] = 0;                        //  terminate the string
  len++;
  }
  else                                        //  too many characters to fit in dataString
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("data string too long."));
    #endif
    return 1;
  }

  // print out the dataString to verify its format
  #if GLACIERPROBE_DEBUG == 1
    for(int i = 0; i<sizeof(dataString); i++){
      USB.print(dataString[i]);
    }
  #endif


  //  Step 2: 
  //  initialize the SD card
  if(!SD.ON())                                //  if the SD card fails to turn on
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Failed to init SD"));
    #endif
    return 2;
  }

  #if GLACIERPROBE_DEBUG == 1
    USB.println(filename);
  #endif
  
  //  Step 3:
  //  Check if file exists, if not then create it, open the file
  if(SD.isFile(filename)==-1)
  {
    SD.create(filename);
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("File created"));
    #endif
  }

  //  Step 4:
  //  Append dataString to the end of the file on a new line.
  if(SD.appendln(filename, dataString))
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("SD append success"));
    #endif
  }
  else
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("SD append failure"));
      for(int i=0; i<sizeof(SD.buffer); i++)            // print out the error message stored in the SD object
      {
        USB.print(SD.buffer[i]);
      }
    #endif
    
    SD.OFF();
    return 3;
  }

  //  Step 5:
  //  Close the file and turn off the SD card
  SdFile* currDir = &SD.currentDir;                     //  the directory to the file is stored as an SdFile object
  uint8_t error = SD.closeFile(currDir);                //  closeFile takes the SdFile object, not the character array
  
  
  if(error != 1)
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Failed to close directory"));
    #endif

    return 4;
  }
  
  SD.OFF();                                             //  turn of the SD while we don't use it

  #if GLACIERPROBE_DEBUG == 1
    USB.println(F("SD write completed, directory closed"));
  #endif
  
  return 0;
}

/*
checkTime()
Updates the data interval dynamically, so if the interval is supposed to be 60 seconds but
it has been 62 seconds since the last cycle then it will reduce the interval to 58. Also
updates the seconds variable. Also checks the date to see if it has changed, and if so returns
1 to indicate that the file should transmitted and a new file should be created.

Parameters:
- uint16_t interval: the active data interval based on the constant user parameter.
- uint16_t seconds: the timestamp in seconds
Returns:
- 0 if the date is the same
- 1 if the date changed
 */
 
bool updateTimes(char* seconds, char wtoStr [12]){
  
  uint32_t lastTime = RTC.hour*3600 + RTC.minute*60 + RTC.second; //  previous seconds of the day
  uint8_t lastDay = RTC.day;
  
  RTC.getTime();                                                  //  update the time variables

  //get the new time
  uint32_t currentTime = RTC.hour*3600 + RTC.minute*60 + RTC.second;
  sprintf(seconds, "%u", currentTime);  //  5 chars max, update the seconds character array

  //update the wakeTime interval
  timestamp_t wto;
  RTC.breakTimeOffset(DATA_INTERVAL, &wto);
  
  sprintf_P(wtoStr, PSTR("%.2u:%.2u:%.2u:%.2u"), wto.date, wto.hour, wto.minute, wto.second);
  
  if(RTC.day != lastDay){               //  if the day has changed, the seconds are going to be off anyways
    return 1;                           //  so just return 1 and don't update the interval time
  }

  return 0;                              // the day didn't change
  
}

#endif

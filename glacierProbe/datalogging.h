#ifndef DATALOGGING_H
#define DATALOGGING_H

#include "header.h"

void setFileName(char filename [16])
{
  sprintf(filename, "%.2u-%.2u-%.2u.csv", RTC.year,RTC.month,RTC.day);
}

uint8_t writeDataSet(keyvalue* kvs, uint8_t numPairs)
{
  const uint8_t DATASIZE = 255;

  //  Step 1:
  //  set up the datastring to be written to the file
  char dataString [DATASIZE];
  memset(dataString, 0, DATASIZE);
  uint16_t len = 0;
  
  for(int pairInd = 0; pairInd < numPairs; pairInd++)
  {
    //append the key string to the datastring
    uint8_t size = sizeof(kvs[pairInd].key);
    USB.printf("Size of key%d: %u.\n",pairInd,size);
    char* k = kvs[pairInd].key;
    uint8_t index = 0;
    if(len + size < DATASIZE){
      while( index < size && k[index] != 0){
        dataString[len] = k[index];
        len++;
        index++;
      }
    }
    else{
      USB.println(F("data string overflow."));
      return 0;
    }

    dataString[len] = '=';
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
      USB.println(F("data string overflow."));
      return 0;
    }
    USB.printf("PairIndex: %u.\n",pairInd);
    if(pairInd < numPairs-1)
    {
      
      dataString[len] = ',';
      len++;
    }
  }
  if(len < DATASIZE-1)
  {
  dataString[len] = ';';
  len++;
  dataString[len] = 0;
  len++;
  }
  else
  {
    USB.println(F("data string too long."));
    return 0;
  }
  

  for(int i = 0; i<sizeof(dataString); i++){
    USB.print(dataString[i]);
  }
  //  Step 3:
  //  Get the filename
  if(!SD.ON())
  {
    USB.println(F("Failed to init SD"));
    return 0;
  }
  
  char fname [64];
  setFileName(fname);
  USB.println(fname);

  //  Step 4:
  //  Check if file exists, if not then create it, open the file
  if(SD.isFile(fname)==-1)
  {
    SD.create(fname);
    USB.println(F("file created"));
  }
  USB.printf("SDisFile: %d.\n",SD.isFile(fname));

  //  Step 5:
  //  Append the datastring to the end of the file on a new line.
  if(SD.appendln(fname, dataString))
  {
    USB.println(F("SD append success"));
  }
  else
  {
    USB.println(F("SD append failure"));
    for(int i=0; i<sizeof(SD.buffer); i++){
      USB.print(SD.buffer[i]);
    }
    SD.OFF();
    return 0;
  }

  SD.showFile(fname);
  SdFile* currDir = &SD.currentDir;
  uint8_t error = SD.closeFile(currDir);
  if(error != 1)
  {
    USB.println(F("Failed to close directory"));
  }
  SD.OFF();
  return 1;
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
 
bool updateTime(uint16_t* interval, char* seconds){
  
  uint32_t lastTime = RTC.hour*3600 + RTC.minute*60 + RTC.second; //  previous seconds of the day
  uint8_t lastDay = RTC.day;
  
  RTC.getTime();                                                  //  update the time variables

  //get the new time
  uint32_t currentTime = RTC.hour*3600 + RTC.minute*60 + RTC.second;
  sprintf(seconds, "%u", currentTime);  //  5 chars max, update the seconds character array
  
  if(RTC.day != lastDay){               //  if the day has changed, the seconds are going to be off anyways
    return 1;                           //  so just return 1 and don't update the interval time
  }

  //  if there is a difference between the desired interval and the actual interval, update the time between
  //  taking samples.
  if( (currentTime - lastTime) != DATA_INTERVAL )
  {
    *interval = *interval - ( (int)(currentTime - lastTime) - DATA_INTERVAL );
  }

  
  return 0;                              // the day didn't change
  
}

#endif

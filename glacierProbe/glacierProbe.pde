#include "header.h"
           
my4G comms("gprs.swisscom.ch", "gprs", "gprs");

void setup(){
  USB.ON();
  RTC.ON();
  RTC.getTime();
  lastDate = RTC.date;
  setFileNames(SD_filename, sizeof(SD_filename), FTP_filename, sizeof(FTP_filename));
}

void loop(){
  uint8_t result = checkUnsentFiles();
  if( result == 4 )
  {
    appendUnsentFile();
  }

  
  char wtoStr [12] = {0};
  bool newFile = updateTimes(currData[KV_SECONDS].val, wtoStr);
  
  readAllSensors(currData);
  
  setFileNames(SD_filename, sizeof(SD_filename), FTP_filename, sizeof(FTP_filename));

  writeDataSet(currData, NUM_KEYVALS, SD_filename);
  PWR.deepSleep(wtoStr, RTC_OFFSET, RTC_ALM1_MODE4);
  USB.ON();
  RTC.ON();
}


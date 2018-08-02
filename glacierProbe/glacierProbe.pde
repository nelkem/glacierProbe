#include "header.h"
           
my4G comms("gprs.swisscom.ch", "gprs", "gprs");

void setup(){
  USB.ON();
}

void loop(){
  char wtoStr [12];
  bool newFile = updateTimes(currData[KV_SECONDS].val, wtoStr);
  
  readAllSensors(currData);

  if(newFile == 1){
    comms.postFTP(FTP_SERVER, FTP_PORT, FTP_USER, FTP_PASS, SD_filename, FTP_filename);
  }
  
  setFileNames(SD_filename, sizeof(SD_filename), FTP_filename, sizeof(FTP_filename));
  
  writeDataSet(currData, 7, SD_filename);
  PWR.deepSleep(wtoStr, RTC_OFFSET, RTC_ALM1_MODE4);
  USB.ON();
  RTC.ON();
}


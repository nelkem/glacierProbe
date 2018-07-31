#include "header.h"
           
my4G comms("gprs.swisscom.ch", "gprs", "gprs");

void setup(){
  USB.ON();
}

void loop(){
  uint32_t firstTime = millis();
  updateTime(&dataInterval, currData[KV_SECONDS].val);
  readAllSensors(currData);
  uint8_t ans = writeDataSet(currData, 7);
  USB.printf("\n\nAns: %u.\n\n",ans);
  uint32_t secondTime = millis();
  USB.printf("\nIt took %u milliseconds.\n", secondTime - firstTime);
  delay(dataInterval * 1000);
}


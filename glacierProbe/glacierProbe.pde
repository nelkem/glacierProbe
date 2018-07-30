#include "header.h"

keyvalue currData[] = {keyvalue("temperature"),
                       keyvalue("humidity"),
                       keyvalue("pressure"),
                       keyvalue("sonic"),
                       keyvalue("wetness"),
                       keyvalue("solar")};
                       
my4G comms;

void setup(){
  USB.ON();
}

void loop(){
  readAllSensors(currData);
  USB.printf("\nData:\n");
  for(int i=0; i<6; i++){
  	USB.printf("%s = %s.\n",currData[i].key, currData[i].val);
  }
  char name [] = "glacierProbe";
  uint8_t a = comms.sendDweet(80, name, sizeof(name), currData, 6);
  USB.printf("\nError: %u\n", a);
  delay(60000);
}


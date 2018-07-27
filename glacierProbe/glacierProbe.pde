#include "header.h"

#if _BME
keyvalue currTemperature = 	{ .key = "temp"};		//	initialize keyvalue objects for each measurement
keyvalue currHumidity = 	{ .key = "humidity"};
keyvalue currPressure = 	{ .key = "pressure"};

currData[0] = &currTemperature;
currData[1] = &currHumidity;
currData[2] = &currPressure;
#endif

#if _SONIC
keyvalue currSonic = { .key = "sonic"};

currData[3] = &currSonic;
#endif

#if _PHYTOS
keyvalue currWetness = { .key = "wetness"};

currData[4] = &currWetness;
#endif

#if _SOLAR
keyvalue currSolar = { .key = "solar"};

currData[5] = &currSolar;
#endif



void setup(){

}

void loop(){
  readAllSensors(currData);
  USB.printf("\nData:\n");
  for(int i=0; i<6; i++){
  	USB.printf("%s = %s.\n",currData[i]->key, currData[i]->val);
  }
}


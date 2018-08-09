#ifndef SENSORS_H
#define SENSORS_H

#include "header.h"
/******************************************************************************************
Sensors.h
Author:	Mitch Nelke
Date:	Thursday July 26, 2018

This file contains code for the sensors our glacier probe will be using. The functions
do not return the measurements, but instead convert them to character arrays and store them
in a user-passed character array. This simplifies the process of storing them in keyvalue
objects like so:

keyvalue temperature;
temperature.key = "temp";
readBME(temperature.val);

******************************************************************************************/

void readAllSensors( keyvalue*);
void cleanString(char*, uint8_t);

/*
readBME()

Reads the BME280 temperature, humidity, and pressure sensor's measurements. Stores them as
character arrays 10 bytes long, so they will have a few 0's in front of the actual value.
*/

#if _BME == 1
bme bme280(AGR_XTR_SOCKET_A);			//	initialize a bme object


void readBME( char* value1,				//	character arrays the function stores the measurements in
			        char* value2, 				//	val1 is t, val2 is h, val3 is p
			        char* value3,
			        uint8_t size)		
{
	memset( value1, 0, size);	//	clears the array before storing data
	memset( value2, 0, size);
	memset( value3, 0, size);

//	start up the sensor, wait a bit, and then grab the t, h, and p readings as floats.
	bme280.ON();
	float t = bme280.getTemperature();
	float h = bme280.getHumidity();
	float p = bme280.getPressure();
	bme280.OFF();

//	convert the floats to 10-byte-wide, 3-decimal-place character arrays and store them in
//	the designated character arrays.
	dtostrf(t, 10, 3, value1);
	dtostrf(h, 10, 3, value2);
	dtostrf(p, 10, 3, value3);
}
#endif


/*
readSonic()

Reads the ultrasonic sensor's distance measurement. Stores the measurement as a character array
10 bytes long, so it will have a few 0's in front of the actual value. The "value" parameter is
a character array that the distance value will be stored in.
*/

#if _SONIC == 1
ultrasound sonic(AGR_XTR_SOCKET_D);		          //	initialize an ultrasound object

void readSonic( char* value, uint8_t size)			//	value is the char array that will store the distance
{			
	memset( value, 0, size);	                    //	clears the array before storing data

//	start up the sensor, wait a bit, then grab the distance measurement as an unsigned integer
	sonic.ON();
	uint16_t d = sonic.getDistance();
	sonic.OFF();

//	convert the unsigned integer to a character array and store it in value
	snprintf(value, 10, "%u", d);
}
#endif


/*
readPhytos()

Reads the leaf wetness sensor's wetness measurement. Stores the wetness as a character array 10
bytes long, so it will have a few 0's in front of the actual value. The "value" parameter is a
character array that the wetness value will be stored in.
*/

#if _PHYTOS == 1
leafWetness phytos;						//	initialize a leafWetness object


void readPhytos( char* value, uint8_t size)			//	value is the char array that will store the wetness
{
	memset( value, 0, size);	//	clears the array before storing data

//	start up the sensor, wait a bit, then take a measurement. The wetness is stored as a member of
//	the object, rather than being returned.
	phytos.ON();
	phytos.read();
	phytos.OFF();

	float w = phytos.wetness;			//	grab the wetness from the member variable of the object
	dtostrf(w, 10, 3, value);			//	convert the float to a character array and store it in value
}
#endif


/*
readSolar()

Reads the solar radiation sensor's intensity measurement. Stores the radiation as a character array
10 bytes long, so it will have a few 0's in front of the actual value. The "value" parameter is a
character array that the radation value will be stored in.
*/

#if _SOLAR == 1
Apogee_SQ110 solar = Apogee_SQ110(AGR_XTR_SOCKET_F);	//	initialize an Apogee_SQ110 object


void readSolar( char* value, uint8_t size)			//	value is the char array that will store the radation
{
	memset( value, 0, size);	//	clears the array before storing data

//	start up the sensor, wait a bit, then take a measurement. The radiation is stored as a member of
//	the object, rather than being returned.
	solar.ON();
	solar.read();
	solar.OFF();

	float r = solar.radiationVoltage;			//	grab the radiation from the member variable of the object
	dtostrf(r, 10, 3, value);			//	convert the float to a character array aand store it in value
}
#endif

#if _DS2 == 1
DS2 ds2(AGR_XTR_SOCKET_C);
#endif


void readAllSensors( keyvalue* dataArray){
	#if _BME
	readBME(dataArray[KV_TEMPERATURE].val, 
			    dataArray[KV_HUMIDITY].val, 
			    dataArray[KV_PRESSURE].val,
			    dataArray[KV_TEMPERATURE].KEYVAL_STRING_SIZE);
	#endif

	#if _SONIC
	readSonic(  dataArray[KV_SONIC].val,
	            dataArray[KV_SONIC].KEYVAL_STRING_SIZE);
	#endif

	#if _PHYTOS
	readPhytos( dataArray[KV_WETNESS].val,
	            dataArray[KV_WETNESS].KEYVAL_STRING_SIZE);
	#endif

	#if _SOLAR
	readSolar(  dataArray[KV_SOLAR].val,
	            dataArray[KV_SOLAR].KEYVAL_STRING_SIZE);
	#endif

  #if _DS2
  ds2.ON();
  ds2.read();

  uint8_t size = dataArray[KV_UBAR].KEYVAL_STRING_SIZE;
  
  ds2.get_ubar( dataArray[KV_UBAR].val, size);
                
  ds2.get_vbar( dataArray[KV_VBAR].val, size);

  ds2.getGust(  dataArray[KV_GUST].val, size);

  ds2.getWindSpeed(     dataArray[KV_WINDSPEED].val, size);

  ds2.getWindDirection( dataArray[KV_WINDDIRECTION].val, size);

  ds2.getTemperature(   dataArray[KV_DS2TEMPERATURE].val, size);
  ds2.OFF();
  #endif

  for(int i = 0; i<NUM_KEYVALS; i++)
  {
    cleanString(dataArray[i].val, sizeof(dataArray[i].val));
  }

}

/*
cleanString()

Takes a character array and its size and replaces it with a version that doesn't include any spaces in front of it.
*/

void cleanString(char* str, uint8_t size)
{
  char newStr [size] = {};
  
  uint8_t index = 0;
  bool started = false;
  
  for(int i = 0; i<size; i++)
  {
    if(started == false){
      if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n'){
        started = true;
        newStr[index] = str[i];
        index++;
      }
    }
    else
    {
      newStr[index] = str[i];
      index++;
    }
  }
  memcpy(str,0,size);
  
  strcpy(str,newStr);
}

#endif


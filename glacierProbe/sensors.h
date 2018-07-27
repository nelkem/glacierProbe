#pragma once							//	include guard

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

#define DELAYTIME 	20					//	milliseconds between sensor.ON and measurement

#define _BME 		0					//	set to 1 if BME sensor is attached
#define _SONIC 		0					//	set to 1 if SONIC sensor is attached
#define _PHYTOS 	0					//	set to 1 if PHYTOS sensor is attached
#define _SOLAR 		0					//	set to 1 if SOLAR sensor is attached


/*
readBME()

Reads the BME280 temperature, humidity, and pressure sensor's measurements. Stores them as
character arrays 10 bytes long, so they will have a few 0's in front of the actual value.
*/

#if _BME == 1
bme bme280(AGR_XTR_SOCKET_A);			//	initialize a bme object


void readBME(char* value1,				//	character arrays the function stores the measurements in
			 char* value2, 				//	val1 is t, val2 is h, val3 is p
			 char* value3)			
{
	memset( values1, 0, sizeof(values1));	//	clears the array before storing data
	memset( values2, 0, sizeof(values2));
	memset( values3, 0, sizeof(values3));

//	start up the sensor, wait a bit, and then grab the t, h, and p readings as floats.
	bme280.ON();
	delay(DELAYTIME);
	float t = bme280.getTemperature();
	float h = bme280.getHumidity();
	float p = bme280.getPressure();
	bme280.OFF();

//	convert the floats to 10-byte-wide, 3-decimal-place character arrays and store them in
//	the designated character arrays.
	dtostrf(t, 10, 3, values1);
	dtostrf(h, 10, 3, values2);
	dtostrf(p, 10, 3, values3);
}
#endif


/*
readSonic()

Reads the ultrasonic sensor's distance measurement. Stores the measurement as a character array
10 bytes long, so it will have a few 0's in front of the actual value. The "value" parameter is
a character array that the distance value will be stored in.
*/

#if _SONIC == 1
ultrasound sonic(AGR_XTR_SOCKET_D);		//	initialize an ultrasound object

void readSonic( char* value)			//	value is the char array that will store the distance
{			
	memset( value, 0, sizeof(value));	//	clears the array before storing data

//	start up the sensor, wait a bit, then grab the distance measurement as an unsigned integer
	sonic.ON();
	delay(DELAYTIME);
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


void readPhytos( char* value)			//	value is the char array that will store the wetness
{
	memset( value, 0, sizeof(value));	//	clears the array before storing data

//	start up the sensor, wait a bit, then take a measurement. The wetness is stored as a member of
//	the object, rather than being returned.
	phytos.ON();
	delay(DELAYTIME);
	phytos.read();
	phytos.OFF();

	float w = phytos.wetness;			//	grab the wetness from the member variable of the object
	dtostrf(w, 10, 3, value)			//	convert the float to a character array and store it in value
}
#endif


/*
readSolar()

Reads the solar radiation sensor's intensity measurement. Stores the radiation as a character array
10 bytes long, so it will have a few 0's in front of the actual value. The "value" parameter is a
character array that the radation value will be stored in.
*/

#if _SOLAR == 1
Apogee_SQ110 solar(AGR_XTR_SOCKET_F);	//	initialize an Apogee_SQ110 object


void readSolar( char* value)			//	value is the char array that will store the radation
{
	memset( value, 0, sizeof(value));	//	clears the array before storing data

//	start up the sensor, wait a bit, then take a measurement. The radiation is stored as a member of
//	the object, rather than being returned.
	solar.ON();
	delay(DELAYTIME);
	solar.read();
	solar.OFF();

	float r = solar.radiation;			//	grab the radiation from the member variable of the object
	dtostrf(r, 10, 3, value);			//	convert the float to a character array aand store it in value
}
#endif


void readAllSensors( keyvalue* dataArray [6]){
	#if _BME
	readBME(dataArray[0]->val, 
			dataArray[1]->val, 
			dataArray[2]->val);
	#endif

	#if _SONIC
	readSonic(dataArray[3]->val);
	#endif

	#if _PHYTOS
	readPhytos(dataArray[4]->val);
	#endif

	#if _SOLAR
	readSolar(dataArray[5]->val);
	#endif

}


#pragma once

/******************************************************************************************

DS2.H

This is the header file for the DS2 sonic anemometer sensor library. It is written to interface
with the Libelium Plug N Sense Ag Pro Xtr datalogger and its libraries. Much of it is
based on Libelium's source code, and is written to act like any other sensor they supported.

Author:		Mitch Nelke, OPEnS Lab OSU
Date:		Aug 7, 2018

******************************************************************************************/

#include <WaspSensorAgrXtr.h>

#define DS2_DEBUG 1

class DS2: public WaspSensorAgrXtr
{

private:
	typedef WaspSensorAgrXtr super;

	const static uint8_t strSize = 8;

	char ubar[strSize];					//	meridial average velocity in m/s
	char vbar[strSize];
	char gust[strSize];					//	max gust speed since last querry
	char windSpeed[strSize];			//	current wind speed
	char windDirection[strSize];		//	current wind direction
	char temperature[strSize];			//	current temperature
	char responseBuffer[40];

	bool compChecksum();				//	calculates the expected checksum based on ubar, vbar, and gust

public:
	DS2(uint8_t socket);				//	constructor with parameter for what socket it's attached to


	//	getters take a character array and its length and store the string representing the measurement
	//	inside it. When the my4G class is included, this would be a keyvalue object's val and
	//	KV_STRING_SIZE members.
	bool get_ubar(char*, uint8_t);			
	bool get_vbar(char*, uint8_t);
	bool getGust(char*, uint8_t);			
	bool getWindSpeed(char*, uint8_t);		
	bool getWindDirection(char*, uint8_t);	
	bool getTemperature(char*, uint8_t);		

	uint8_t read();						//	reads all measurements from the DS2 and stores them in their variables
	bool sendCommand(char*, uint8_t);	//	used for generically sending commands and storing the response in
										//	responseBuffer

	uint8_t ON();
	void OFF();



};
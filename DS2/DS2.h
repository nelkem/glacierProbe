#pragma once

#include <WaspSensorAgrXtr.h>

#define DS2_DEBUG 0

class DS2: public WaspSensorAgrXtr
{

private:
	typedef WaspSensorAgrXtr super;

	const uint8_t strSize = 7;

	char ubar[strSize];					//	meridial average velocity in m/s
	char vbar[strSize];
	char gust[strSize];					//	max gust speed since last querry
	char windSpeed[strSize];				//	current wind speed
	char windDirection[strSize];			//	current wind direction
	char temperature[strSize];			//	current temperature

	char checksum;

	char calcChecksum();

public:
	DS2(uint8_t socket);			//	constructor with parameter for what socket it's attached to


	//	getters take a character array and its length and store the string representing the measurement
	//	inside it. When the my4G class is included, this would be a keyvalue object's val and
	//	KV_STRING_SIZE members.
	bool get_ubar(char*, uint8_t);			
	bool get_vbar(char*, uint8_t);
	bool getGust(char*, uint8_t);			
	bool getWindSpeed(char*, uint8_t);		
	bool getWindDirection(char*, uint8_t);	
	bool getTemperature(char*, uint8_t);		

	uint8_t read();					//	reads all measurements from the DS2 and stores them in their variables

	uint8_t ON();
	void OFF();

};
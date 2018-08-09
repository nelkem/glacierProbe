/******************************************************************************************

DS2.CPP

This is the cpp file for the DS2 sonic anemometer sensor library. It is written to interface
with the Libelium Plug N Sense Ag Pro Xtr datalogger and its libraries. Much of it is
based on Libelium's source code, and is written to act like any other sensor they supported.

Author:		Mitch Nelke, OPEnS Lab OSU
Date:		Aug 7, 2018

******************************************************************************************/

#include "DS2.h"

DS2::DS2(uint8_t socket)
{
	//store sensor location
	_socket = socket;
	if(bitRead(AgricultureXtr.socketRegister, _socket) == 1)
	{
		//Redefinition of socket by two sensors detected
		AgricultureXtr.redefinedSocket = 1;
	}
	else
	{
		bitSet(AgricultureXtr.socketRegister, _socket); 
	}

	this->address = '0';	//	this is the default address of DS-2's, change it if necessary

}

/******************************************************************************************

ON

Powers the device and checks for a response to a general information request. If the device
is responsive and coherent, then it is kept on. If it doesn't respond or is incoherent, the
communication line and power are shut off.

Returns:
0:	No error, device turned on and successfully communicated
1:	Wrong socket
2:	Device unresponsive

******************************************************************************************/

uint8_t DS2::ON()
{
	char message[70];
	if(AgricultureXtr.redefinedSocket == 1)
	{
		//"WARNING: Redefinition of sensor socket detected"
		strcpy_P(message, PSTR("WARNING: REDEF OF SENSOR SOCKET")); 
		PRINTLN_AGR_XTR(message);
	}

	if((_socket == AGR_XTR_SOCKET_E) || (_socket == AGR_XTR_SOCKET_F))
	{
		//"WARNING - The following sensor can not work in the defined socket:"
		strcpy_P(message, PSTR("WARNING: CAN'T WORK IN SOCKET: ")); 
		PRINT_AGR_XTR(message);
		#if DS2_DEBUG == 1
			USB.println(F("SF-421"));
		#endif

		return 1;
	}

	#if DS2_DEBUG == 1
		USB.println(F("Turning DS2 ON..."));
	#endif
	super::ON();		//SDI12 needs both 3v3 and 5v
	set12v(_12V_ON);

	delay(300);			//same delay after powering sensor as Apogee SF421, may not be necessary


	#if DS2_DEBUG == 1
		USB.println(F("Setting Mux"));
	#endif
	setMux();

	//	basically a copy of the isSensor code, but prints out the response instead of checking if the sensor is supported
	strcpy_P(command, PSTR("?I!") );
	sdi12.sendCommand(command, strlen(command));

	#if DS2_DEBUG == 1
		USB.println(F("Command Sent. Receiving response..."));
	#endif
	sdi12.readCommandAnswer(33, LISTEN_TIME);	//	we expect a long response of information on the sensor

	delay(30);

	if(sdi12.available() >= 20)
	{
		//	print out the response and return errorless
		#if DS2_DEBUG == 1
			USB.print(F("DS2 Response: "));
			USB.flush();
			while(sdi12.available())
			{
				USB.print((char)sdi12.read());
				delay(10);
			}
			USB.println();
		#endif
		return 0;
	}

	//	otherwise say the device is nonresponsive and turn it off
	#if DS2_DEBUG == 1
		USB.flush();
		USB.println(F("DS2 Unresponsive."));
		USB.flush();
		USB.printf("%u\n",sdi12.available());
	#endif

	sdi12.setState(DISABLED);
	digitalWrite(MUX_EN, HIGH);
	this->OFF();

	return 2;
}

/******************************************************************************************

OFF

Turns 12v power to the device off, then turns off the socket.

******************************************************************************************/

void DS2::OFF()
{
	set12v(_12V_OFF);		//	turning 12v off requries 3v3
	super::OFF();			//	turn the rest of it off
}

/******************************************************************************************

Getters

All getter functions take in a character array and its size. They clear the array,
copy the respective variable to it, and return.

Returns:
0:	Failed to copy, arary size too small
1:	Copied the value successfully

******************************************************************************************/

bool DS2::get_ubar(char* array, uint8_t size)
{
	memset(array, 0, size);

	if(size > strlen(ubar))
	{
		strcpy(array, ubar);
		return 1;
	}

	return 0;
}

bool DS2::get_vbar(char* array, uint8_t size)
{
	memset(array, 0, size);

	if(size > strlen(vbar))
	{
		strcpy(array, vbar);
		return 1;
	}
	
	return 0;
}

bool DS2::getGust(char* array, uint8_t size)
{
	memset(array, 0, size);

	if(size > strlen(gust))
	{
		strcpy(array, gust);
		return 1;
	}
	
	return 0;
}

bool DS2::getWindSpeed(char* array, uint8_t size)
{
	memset(array, 0, size);

	if(size > strlen(windSpeed))
	{
		strcpy(array, windSpeed);
		return 1;
	}
	
	return 0;
}

bool DS2::getWindDirection(char* array, uint8_t size)
{
	memset(array, 0, size);

	if(size > strlen(windDirection))
	{
		strcpy(array, windDirection);
		return 1;
	}
	
	return 0;
}

bool DS2::getTemperature(char* array, uint8_t size)
{
	memset(array, 0, size);

	if(size > strlen(temperature))
	{
		strcpy(array, temperature);
		return 1;
	}
	
	return 0;
}

/******************************************************************************************

COMPARE CHECKSUM

Sums the values of the characters in responseBuffer (null chars would be 0), ignoring the
device address and the character after '_', which represents the checksum passed by the device.
The DS-2 always appends a checksum to its response to the aR3! command, but most commands can
be modified to request a checksum as well.

Returns:
0:	Calculated and received checksum are not equal
1:	Checksums are equal, message not corrupt

******************************************************************************************/

bool DS2::compChecksum()
{
	uint16_t crc = 0;							//	storing the sum
	int i = 1;									//	index
	char c;

	do
	{
		c = responseBuffer[i];					//	store the bit to add
		crc += (uint8_t) c;						//	add its value to the sum
		i++;									
	}	while(	c != 0 &&						//	while the current bit isnt null
				c != '_' &&						//	the underscore bit comes right before the checksum
				i < sizeof(responseBuffer) );	//	and we aren't overflowing
	crc = crc % 64 + 32;

	//	the next bit should either be the checksum or null

	if(crc == responseBuffer[i])				//	if it's the checksum, compare
	{
		return 1;
	}


	//	if the calculated checksum wasn't equal to the given, or the given was null
	#if DS2_DEBUG == 1
		USB.println(F("CHECKSUM FAILED"));
	#endif
	return 0;
}

/******************************************************************************************

SEND COMMAND

Takes a command and the expected length of the response and sends it to the DS-2. The function
prepends the DS-2's address to every command. The response of the device is stored in the 
responseBuffer member variable and can be accessed until it is cleared. The response is
printed out of debug mode is on.

Returns:
0:	No response and/or invalid command format
1:	Command sent, device responded

******************************************************************************************/


bool DS2::sendCommand(char* cmd, uint8_t length)
{

	memset(responseBuffer, 0, sizeof(responseBuffer));

	if(strlen(cmd) >= 5)
	{
		return 0;
	}
	char fullCommand [6] = {};								//	stores both the user command and the address
	snprintf(fullCommand,sizeof(fullCommand), "%c%s",address,cmd);	//	combine the user command and address
	sdi12.sendCommand(fullCommand, strlen(fullCommand));	//	pass it to the DS-2
	sdi12.readCommandAnswer(length, LISTEN_TIME);			//	receive the response and store it in the buffer

	int i = 0;
	while(	sdi12.available() &&							//	if there are bytes to be read
			i < sizeof(responseBuffer) - 1)					//	and the responseBuffer won't overflow
	{														//	and still has space for a null terminator

		responseBuffer[i] = sdi12.read();					//	read a byte and store it in the buffer
		i++;

	}
	responseBuffer[i] = 0;								//	append a null terminator

	if(i==0)											//	if there was no response, say so and return 1
	{
		#if DS2_DEBUG == 1
			USB.println(F("No response."));
		#endif

		return 0;
	}

	#if DS2_DEBUG == 1									//	otherwise print the response and return 0
		USB.printf("Response: %s\n", responseBuffer);
	#endif

	return 1;

}


/******************************************************************************************

READ

This is the bread and butter of the DS-2 Class. It asks the device for all measurements, 
then stores them in member variables. There are lots of variables so it uses a switch-case
to cycle through them. Measurements are split between two commands. The function receives
a checksum and verifies it.

Returns:
0:	Everthing is A-OKAY, measurements were stored and checksum verified correct
1:	The device failed to respond to the measurement (aM!) command
2:	An unusual number of sensors are available, could mean corrupt message
3:	The device failed to respond to the first data request aD0!
4:	Failed to parse device's response to first data request
5:	The device failed to respond to the second data request aR3!
6:	Failed to parse device's response to the second data request
7:	Checksum failed, data could be corrupt

******************************************************************************************/

uint8_t DS2::read()
{
	//	empty all the strings that will store the measurements
	memset(ubar, 0, strSize);
	memset(vbar, 0, strSize);
	memset(gust, 0, strSize);
	memset(windSpeed, 0, strSize);
	memset(windDirection, 0, strSize);
	memset(temperature, 0, strSize);

	memset(timeToNextMeasure, 0, sizeof(timeToNextMeasure));	//	sensor will say how long to wait

	if( this->sendCommand("M!", 5)  ==  0 )				//	sends the measurement command
	{
		return 1;										//	return if the device is unresponsive
	}

	timeToNextMeasure[0] = responseBuffer[1];			//	get the time to wait before asking for data
	timeToNextMeasure[1] = responseBuffer[2];
	timeToNextMeasure[2] = responseBuffer[3];

	numberOfMeasures = responseBuffer[4];

	if(numberOfMeasures != '3')							//	we expect 3 measurements to be available
	{
		#if DS2_DEBUG == 1
			USB.println(F("Not all sensors are available."));
		#endif
		return 2;
	}

	uint8_t time = atoi(timeToNextMeasure);				//	wait until the sensor can provide data
	delay(time*1000 + 10);

	if( this->sendCommand("D0!", 30) == 0 )				//	ask for the first set of data, return if unresponsive
	{
		return 3;
	}

	uint8_t counter = 0;							//	keeps track of which variable we are parsing
	uint8_t bufIdx = 0;								//	track the index of the responseBuffer
	uint8_t charIdx = 0;							//	track the index of the storage array

	while(	bufIdx < strlen(responseBuffer) &&		//	haven't exceeded the range of responseBuffer
			charIdx < this->strSize - 1 &&			//	haven't exceeded the range of the storage array
			counter < 4)							//	we are storing into three variables
	{
		char c = responseBuffer[bufIdx];			//	snatch the next character to use

		if( c == '+' || c == '-' )					//	in this case, + and - come before the variables
		{											//	so we use them as delimiters AND store them
			counter++;								//	change which variable we are storing into
			charIdx = 0;
		}
		if( c != '\n' )
		{
			switch(counter)								//	based on counter, store char into specified variable
			{
				case 1:
					windSpeed[charIdx] = c;				//	set the character
					charIdx ++; 						//	then increment the index and add a null terminator
					windSpeed[charIdx] = 0;				//	which will be replaced if counter doesn't change
					break;

				case 2:
					windDirection[charIdx] = c;
					charIdx ++;
					windDirection[charIdx] = 0;
					break;

				case 3:
					temperature[charIdx] = c;
					charIdx ++;
					temperature[charIdx] = 0;
					break;

				default:
					break;

			}
		}
		bufIdx ++;									//	increment the index of responseBuffer
	}

	if(counter == 0)								//	if no data had been stored, something went wrong
	{
		#if DS2_DEBUG == 1
			USB.println(F("Unexpected data, read() failed."));
		#endif
		return 4;
	}

	//print out the measurements
	#if DS2_DEBUG == 1
		USB.printf("\n---First Measurements---\nWS: %s\nWD: %s\nTemp: %s\n\n", windSpeed, windDirection, temperature);
	#endif

	if( this->sendCommand("R3!", 30) == 0 )			//	send the second data request, return if unresponsive
	{
		return 5;
	}

	// this is mostly all the same as before
	counter = 0;
	bufIdx = 0;
	charIdx = 0;

	while(	bufIdx < strlen(responseBuffer) &&
			charIdx < this->strSize - 1 &&
			counter < 4)
	{
		char c = responseBuffer[bufIdx];

		if( c == ' ' || c == '\t')					//	this time the delimiters are spaces and tabs
		{											//	since only the negative signs are included
			counter++;
			charIdx = 0;
		}
		switch(counter)
		{
			case 1:
				ubar[charIdx] = c;
				charIdx ++;
				ubar[charIdx] = 0;
				break;

			case 2:
				vbar[charIdx] = c;
				charIdx ++;
				vbar[charIdx] = 0;
				break;

			case 3:
				gust[charIdx] = c;
				charIdx ++;
				gust[charIdx] = 0;
				break;
			default:
				break;
		}
		bufIdx++;
	}

	if(counter == 0)								//	if no data was stored, something went wrong
	{
		#if DS2_DEBUG == 1
			USB.println(F("Unexpected data on R3! cmd, read() failed"));
		#endif

		return 6;
	}

	//	print out the measurements
	#if DS2_DEBUG == 1
		USB.printf("\n---Second Measurements---\nubar: %s\nvbar: %s\ngust: %s\n\n", ubar, vbar, gust);
	#endif

	if( compChecksum() == 0 )						//	verify the checksum by calculating and comparing
	{
		#if DS2_DEBUG == 1
			USB.println(F("Checksum Failed. Data could be invalid."));
		#endif

		return 7;
	}

	#if DS2_DEBUG == 1
		USB.println(F("Data collected and stored. Checksum passed. Done reading."));
	#endif
	return 0;
}



















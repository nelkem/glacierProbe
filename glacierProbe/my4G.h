 #pragma once

 #include "Wasp4G.h"
/*
Author: Mitch Nelke
Date: Thursday, July 26, 2018



TODO:
- add sendPacket function that sends an array of keyvalues
- add sendFile function that sends all the data in an entire file
- add sendRecent function that sends data from file that is new since the last transfer




Data is stored as key-value pairs. Each measurement should get its own keyvalue object. The
initializationf or an object for the humidity measurement, for example, would look like:
keyvalue humidity = {"humidity", "99.9"};

These measurements will be stored in a single array for each measurement cycle, representing
a point in time. For 10 minute data, this would be an array of all the measured keyvalues
for time t=0m, 10m, 20m... so on. These arrays would then be stored in a CSV file on the
onboard SD card and extracted when it is time to send data over 4G.
*/


#define DEBUG_MY4G	1

struct keyvalue {
	const static uint8_t KEYVAL_STRING_SIZE = 10;
	char key [KEYVAL_STRING_SIZE];	//name
	char val [KEYVAL_STRING_SIZE];	//data
}


/*
Simple class for some custom functions for the 4G class, rather than inserting them directly
into Libelium's source file for the Wasp4G class.
*/

class my4G : public Wasp4G
{
public:


	my4G(); // constructor

/*
SendCommand()
Takes a command character array/string literal and a couple desired answers and sends them to
the SIM card. This only works with AT commands in the style of "AT+CREG?" or "AT+CREG=0". More
information on how to use this function is available in the documentation.
*/
	uint8_t sendMyCommand(*char command);

	uint8_t sendMyCommand(*char command,
						*char ans1);

	uint8_t sendMyCommand(*char command,
						bool print,
						*char ans1);

	uint8_t sendMyCommand(*char command,// AT command to send to SIM
						bool print;		// TRUE for printing to USB
						*char ans1, 	// Desired answer 1, wi,ll return 1 if this is the response
						*char ans2); 	// Desired answer 2, will return 2 if this is the response


/*
Takes a host url, a port, a resource string, and an array of keyvalue objects and "Dweets" them.
Not really useful for bulk data but nice as a demo and could be reused for other FTP servers.
*/
	uint8_t sendDweet(	char* url, 			//just the host
						uint16_t port 		// usually 80
						char* resource, 	// the location of the host's site the data is going
						(keyvalue*)* data, 	// array of an array of keyvalue pairs.
						uint8_t numPairs	// number of pairs in the data array
						);
};





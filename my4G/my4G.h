 #ifndef MY4G_H
 #define MY4G_H
 
 #include <Wasp4G.h>
 #include "structures.h"
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


/*
Simple class for some custom functions for the 4G class, rather than inserting them directly
into Libelium's source file for the Wasp4G class.
*/

class my4G : public Wasp4G
{
public:


	my4G(	char* apn,			//	apn name
			char* login,		//	apn login
			char* password);	//	apn password

/*
SendCommand()
Takes a command character array/string literal and a couple desired answers and sends them to
the SIM card. This only works with AT commands in the style of "AT+CREG?" or "AT+CREG=0". More
information on how to use this function is available in the documentation.
*/
	uint8_t sendMyCommand(	char*);

	uint8_t sendMyCommand(	char*,
							char*);

	uint8_t sendMyCommand(	char*,	// AT command to send to SIM
							char*, 	// Desired answer 1, wi,ll return 1 if this is the response
							char*); 	// Desired answer 2, will return 2 if this is the response


/*
Takes a host url, a port, a resource string, and an array of keyvalue objects and "Dweets" them.
Not really useful for bulk data but nice as a demo and could be reused for other FTP servers.
*/
	uint8_t sendDweet(  uint16_t port,		   // usually 80
						char* name, 	       // the location of the host's site the data is going
						uint8_t size,
						keyvalue* data,        // array of keyvalue pointers.
						uint8_t numPairs);	   // number of pairs in the data array


/*
Post FTP
Takes ftp settings, the filename stored on the SD card, and the server file to post to. Then sends it over.
*/

	uint8_t postFTP(	char* ftp_server,
						uint8_t ftp_port,
						char* ftp_user,
						char* ftp_pass,
						char* SD_filename,
						char* serverFile);

	void serialCommandMode();

};

#endif


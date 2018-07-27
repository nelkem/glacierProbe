/*
Author: Mitch Nelke
Date: Thursday, July 26, 2018

*/


#include "my4G.h" //check this file for descriptions of functions

my4G::my4G(){}; //nothing is different from the Wasp4G initialization.

uint8_t my4G::sendMyCommand(*char command)
{
	this.sendMyCommand(	command,	//character array representing the AT command
					 	TRUE,		//print out messages to the serial monitor
					 	NULL,		//no desired answer1
					 	NULL);		//no desired answer2
}

uint8_t my4G::sendMyCommand(*char command
							*char ans1)
{
	this.sendMyCommand(	command,	//character array representing the AT command
					 	TRUE,		//print out messages to the serial monitor
					 	ans1,		//desired answer1
					 	NULL);		//no desired answer2
}

uint8_t my4G::sendMyCommand(*char command
							bool print,
							*char ans1)
{
	this.sendMyCommand(	command,	//character array representing the AT command
					 	print,		//print out messages to the serial monitor
					 	ans1,		//desired answer1
					 	NULL);		//no desired answer2
}

uint8_t my4G::sendMyCommand(*char command,
							bool print,
							*char ans1,
							*char ans2)
{
	uint8_t answer;

	if(print) USB.printf("Sending %s to SIM....\n", command);

	answer = sendCommand(myCommand, ans1, ans2); //use UART to send the AT command to the SIM

	if(print)
	{
		USB.printf(F("Response:\t"))
		for(int i=0; i< 512; i++) USB.print((char)_buffer[i]); //print out SIM response
		USB.println(); 
	}
	if(answer == 0){
		if(sizeof(_buffer)) return 3; 	//if there was any response at all.
		else return 0; 					//probably an invalid command or the module is off.
	}

	return answer; //ans1 or ans2 were recognized

}

uint8_t sendDweet(	uint16_t port,
					char* name,
					(keyvalue*)* data,
					uint8_t numPairs)
{
	char resource [64];
	memset(resource, 0, sizeof(resource));
	if(sizeof(resourceString) + sizeof(name) < 63){
		strcpy_P(resource, F("/dweet/for/%s?"), name);
	}
	else{
		#if DEBUG_MY4G
			USB.println("Device name is too long.");
			return 128;
		#endif
	}

	/*
	We need to create the character array for the data. Since we've stored the current
	data measurements in an array of keyvalue object pointers, theres some manipulation involved.
	*/


	char dataString [512];						// 	the string to pass to http() post function.
	memset(dataString, 0, sizeof(dataString));	//	clear dataString. 
	uint8_t len = 0; 							//	length of data string
	uint8_t index = 0;							//	current keyvalue pair in array

	while(index < numPairs &&
		len < 128)
	{

		//Insert the key string into dataString

		uint8_t charIndex = 0;					//	current character index in the key
		char* k = (data[i].key);				//	pointer to first character in key 
		while( 	k[charIndex] != '\0' &&			//	while the current char is not a null terminator
				charIndex <= sizeof( k ) &&		//	and charIndex is not larger than the length of the string
				charIndex < 128)				//	and charIndex is not equal to the max size of dataString
		{
			dataString[len] = k[charIndex];
			len++;
			charIndex++;
		}

		dataString[len] = '=';					//	insert separator between key and value.
		len++;

		//Then insert the val string into dataString

		charIndex = 0;
		char* v = (data[i].val);
		while(	v[charIndex] != '\0' &&
				charIndex <= sizeof( v ) &&
				charIndex < 128)
		{
			dataString[len] = v[charIndex];
			len++;
			charIndex++
		}

		index++;								//	next keyvalue pair

		if(index <= numPairs){					//	if there is another keyvalue pair
			dataString[len] = '&';				//	insert separator between keyvalue pairs.
			len++;
		}
	}

	/*
	Now we can Dweet the data by calling Wasp4G's function for posting http urls.
	*/

	uint8_t postError;
	postError = this.http(this.HTTP_POST,		//	method
							url,				//	host url
							port,				//	port
							resource,			//	resource
							dataString);		//	data

	return postError;							//	between 0 and 27. 0 means OK.

}





































/*
  Author: Mitch Nelke
  Date: Thursday, July 26, 2018

*/

#ifndef __WPROGRAM_H__
#include "WaspClasses.h"
#endif

#include "my4G.h" //check this file for descriptions of functions
#include <inttypes.h>

my4G::my4G(char* apn, char* login, char* password) 
{
  this->set_APN(apn, login, password);
}; //nothing is different from the Wasp4G initialization.



/**************************************************************************************************************
SendMyCommand()
Takes an AT command string and up to two desired answers and sends it to the SIM.
Parameters:
- Char* command: the AT command to send to the SIM, formatted as a string such as "AT+CREG?"
- char* ans1: desired response from SIM
- char* ans2: desired response from SIM
Returns:
- 0 if SIM doesn't respond with ans1 or ans2
- 1 if SIM responds with ans1
- 2 if SIM responds with ans2
- 3 if SIM doesn't respond at all
***************************************************************************************************************/

uint8_t my4G::sendMyCommand(char* command)
{
  this->sendMyCommand(command,	// character array representing the AT command
                      NULL,		  // no desired answer1
                      NULL);		// no desired answer2
}

uint8_t my4G::sendMyCommand(char* command,
                            char* ans1)
{
  this->sendMyCommand(command,	//character array representing the AT command
                      ans1,		  //desired answer1
                      NULL);		//no desired answer2
}

uint8_t my4G::sendMyCommand(char* command,
                            char* ans1)
{
  this->sendMyCommand(command,	//character array representing the AT command
                      ans1,		//desired answer1
                      NULL);		//no desired answer2
}

uint8_t my4G::sendMyCommand(char* command,
                            char* ans1,
                            char* ans2)
{
  uint8_t answer;

  USB.printf("Sending %s to SIM....\n", command);

  answer = sendCommand(command, ans1, ans2);               //use UART to send the AT command to the SIM


  USB.printf("Response:\t");
  for (int i = 0; i < 512; i++) 
  {
    USB.print((char)_buffer[i]); //print out SIM response
  }
  USB.println();

  if (answer == 0) {
    if (_buffer[1] != 0) return 3; 	                      //if there was any response at all.
    else return 0; 					                              //probably an invalid command or the module is off.
  }

  return answer;                                          //ans1 or ans2 were recognized

}




/**************************************************************************************************************
SEND DWEET
Parameters:
- Port: an integer value representing the port
- Name: a character array representing the name of the device, such as "glacierProbe".
- Size: when calling sendDweet, use sizeof(name) in this field. This is because when character arrays are passed
  to the function they are converted to pointers to character arrays, so the pointer's size is just the register
  address.
- Data: an array of keyvalue objects, which store a character array for each the key and the value of a datapoint.
- numPairs: an integer representing the number of data points, such as temperature and pressure, stored in the
  data keyvalue array. You can't easily use sizeof to get this value, you should just keep track of it somehow.

Returns:
- 
***************************************************************************************************************/
uint8_t my4G::sendDweet(	uint16_t port,
                          char* name,
                          uint8_t size,
                          keyvalue* data,
                          uint8_t numPairs)
{
  char resource [64];                                    // init a blank character array to store the url
  memset(resource, 0, sizeof(resource));
  char dweetString [] = "/dweet/for/";                   // init the constant section of the resource
  memcpy(resource, dweetString, sizeof(dweetString)-1);  // copy the constant section to resource
  if (sizeof(dweetString) + size < 63)                   // make sure it won't overflow if the name is added
  {

    // copy over the name. Resource is an array, so its name represents the register address. +sizeof shifts
    // the address over by the number of characters in dweetString. -1 cuts off the null terminator since we
    // used a string literal to initialize dweetString.

    memcpy(resource+sizeof(dweetString)-1,name,size);    // name is the name of the device, size is the length.   
    resource[sizeof(dweetString)+size-2] = '?';          // two null terminators now so -2.
                                                         // '?' transitions from the name to the data
  }

  else {                                                 // If the name was too long, say so and don't continue
    #if DEBUG_MY4G
      USB.println(F("Device name is too long."));
    #endif
    return 128;
  }

  /*
    We need to create the character array for the data. Since we've stored the current
    data measurements in an array of keyvalue object pointers, theres some manipulation involved.
  */
  const uint8_t DATALENGTH = 256;
  char dataString [DATALENGTH];						                     // the data string to pass to the http() function.
  memset(dataString, 0, sizeof(dataString));	           //	clear dataString.
  uint8_t len = 0; 							                         //	used length of data string
  uint8_t pair = 0;							                       //	current keyvalue pair in array

  while (pair < numPairs &&                              // while this is not the last pair
         len < DATALENGTH)                                      // and the length of the datastring is not too long
  {

    //Insert the key string into dataString

    uint8_t charIndex = 0;					            //	current character index in the key
    char* k = data[pair].key;				          //	pointer to first character in key
    uint8_t strSize = sizeof(data[pair].key);
    while ( 	k[charIndex] != '\0' &&			      //	while the current char is not a null terminator
              charIndex <= strSize &&		        //	and charIndex is not larger than the length of the string
              charIndex < DATALENGTH)				          //	and charIndex is not equal to the max size of dataString
    {
      dataString[len] = k[charIndex];
      len++;
      charIndex++;
    }

    dataString[len] = '=';					         //	insert separator between key and value.
    len++;

    //Then insert the val string into dataString

    charIndex = 0;
    char* v = (data[pair].val);
    strSize = sizeof(data[pair].val);
    while (	v[charIndex] != '\0' &&
            charIndex <= strSize &&
            charIndex < DATALENGTH)
    {
      dataString[len] = v[charIndex];
      len++;
      charIndex++;
    }

    pair++;								                 //	next keyvalue pair

    if (pair <= numPairs) {					       //	if there is another keyvalue pair
      dataString[len] = '&';				         //	insert separator between keyvalue pairs.
      len++;
    }
  }
  if(len>256){
    USB.println(F("Data too long."));
    return 129;
  }

  /*
    Now we can Dweet the data by calling Wasp4G's function for posting http urls.
  */
  USB.println(F("POSTING"));
  USB.printf("Resource: %s,\tdataString: %s\n",resource,dataString);
  char host [] = "dweet.io";
  uint8_t postError;

  this->ON();                               //  Turn 4G on

  postError = this->http(HTTP_POST,		      //	method
                        host,				        //	host url
                        port,				        //	port
                        resource,			      //	resource
                        dataString);		    //	data

  this->OFF();
  return postError;							            //	between 0 and 27. 0 means OK.
  

}

/**************************************************************************************************************
Post to FTP
Parameters:

Returns:
***************************************************************************************************************/

uint8_t my4G::postFTP(char* ftp_server,
              uint8_t ftp_port,
              char* ftp_user,
              char* ftp_pass,
              char* SD_file,
              char* serverFile)
{
  this->ON();
  uint8_t error = this->ftpOpenSession(ftp_server,
                                      ftp_port,
                                      ftp_user,
                                      ftp_pass);

  if(error == 0)
  {
    USB.println(F("FTP open session OK"));
    uint32_t previous = millis();

    error = this->ftpUpload(serverFile, SD_file);
    if(error == 0)
    {
      USB.print(F("2.2. Uploading SD file to FTP server done! "));
      USB.print(F("Upload time: "));
      USB.print((millis() - previous) / 1000, DEC);
      USB.println(F(" s"));
    }
    else
    {
      USB.print(F("2.2. Error calling 'ftpUpload' function. Error: "));
      USB.println(error, DEC);
    }

    error = this->ftpCloseSession();
    this->OFF();

    if (error == 0)
    {
      USB.println(F("2.3. FTP close session OK"));
      return 1;
    }
    else
    {
      USB.print(F("2.3. Error calling 'ftpCloseSession' function. error: "));
      USB.println(error, DEC);
      USB.print(F("CMEE error: "));
      USB.println(_4G._errorCode, DEC);
      return 0;
    }
  }
  else
  {
    USB.print(F( "2.1. FTP connection error: "));
    USB.println(error, DEC);
    return 0;
  }
}

/**************************************************************************************************************
serialCommandMode()
Waits for a serial command and then sends it to the SIM card, printing the response. The user can enter 'q'
or 'Q' to quit the loop, but otherwise it will repeat indefinitely.

Parameters: none
Returns: none
***************************************************************************************************************/

void my4G::serialCommandMode(){
  char cmd_buffer [50] = {};
  uint8_t buff_index = 0;
  
  while(true)                                                 //  loop until we force an exit
  {
    USB.print(F("Enter 'q' to quit"));
    USB.print(F("COMMAND: "));

    memcpy(cmd_buffer,0,sizeof(cmd_buffer));                  //  clear the command buffer
    while(USB.available() && buff_index < sizeof(cmd_buffer)) //  if there are characters left in the CMD
    {
      cmd_buffer[buff_index] = USB.read();                    //  copy the byte to the command buffer

      if( cmd_buffer[buff_index] == 'q' ||                    //  exit conditions, return from function
          cmd_buffer[buff_index] == 'Q')
      {
        return;
      }

      buff_index++;
    }
    USB.println(cmd_buffer);

    this->sendMyCommand(cmd_buffer);
  }
}















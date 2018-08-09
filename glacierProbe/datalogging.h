#ifndef DATALOGGING_H
#define DATALOGGING_H

#include "header.h"


/*
setFileNames()
Updates the SD_filename to correspond with the current date, then updates FTP_filename's filename by
appending it after the base directory

returns: 
- 0 if directory is successfully saved
- 1 if directory is not successfully saved

 */

uint8_t setFileNames(char* SD_filename,                    // name of file storing data on SD         
                    uint8_t SD_len,                       //  length of the name of the file
                    char* FTP_filename,                   //  name of file and directory for FTP server
                    uint8_t FTP_length)                   //  length of the FTP server directory
{

//  update the SD filename based on the current date and format settings
  switch(SD_FILEFORMAT){
    case(DAY_MONTH_YEAR):
      sprintf_P(SD_filename, PSTR("%.2u-%.2u-%.2u.csv"), RTC.date,RTC.month,RTC.year);
      break;
    case(YEAR_MONTH_DAY):
      sprintf_P(SD_filename, PSTR("%.2u-%.2u-%.2u.csv"), RTC.year,RTC.month,RTC.date);   
      break;
    case(SINGLE_FILE):
      strcpy(SD_filename, "SENSOR_DATA");
      break;
  }

  if(FTP_length > sizeof(FTP_DIR) + SD_len - 1)             //  check for name overflow
  {
    //  append the filename to the directory for the FTP server
    strcpy(FTP_filename, FTP_DIR);
    strcpy(FTP_filename + sizeof(FTP_DIR) - 1, SD_filename);

    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Directory successfully created"));
    #endif
    return 0;
  }
  
  #if GLACIERPROBE_DEBUG == 1
    USB.println(F("not enough space in FTP_filename"));
  #endif
  return 1;
}

/*
writeDataSet()
Formats a large characterarray dataString representing the key value pairs. Keys are separated from their
values with equal signs, keyval pairs are separated from each other with commas, and cyles of measurements are
separated with semicolons.

returns: 
- 0 if dataSet is successfully saved to SD file
- 1 if directory is not successfully saved
- 2 if SD card fails to initialize
- 3 if SD failed to append data to file
- 4 if SD failed to close directory
*/

uint8_t writeDataSet(keyvalue* kvs, uint8_t numPairs, char* filename)
{
  const uint8_t DATASIZE = 254;                 //  max length of the data string

  //  Step 1:
  //  set up the datastring to be written to the file
  char dataString [DATASIZE] = { 0 };
  uint8_t len = 0;
  
  for(int pairInd = 0; pairInd < numPairs; pairInd++)
  {
    //append the key string to the datastring
    uint8_t size = sizeof(kvs[pairInd].key);
    char* k = kvs[pairInd].key;
    uint8_t index = 0;                          //  index of character array 'k'
    
    if(len + size < DATASIZE){                  //  if the key won't overflow the datastring
      while( index < size && k[index] != 0){    //  making sure indices are in bounds and the character is not null
        dataString[len] = k[index];
        len++;
        index++;
      }
    }
    else{
      #if GLACIERPROBE_DEBUG == 1
        USB.println(F("data string overflow."));
      #endif
      return 1;
    }

    dataString[len] = '=';                      // separate the key from the value
    len++;

    //append the val string to the datastring
    size = sizeof(kvs[pairInd].val);
    char* v = kvs[pairInd].val;
    index = 0;
    if(len + size < DATASIZE){
      while( index < size && v[index] != 0){
        dataString[len] = v[index];
        len++;
        index++;
      }
    }
    else{
      #if GLACIERPROBE_DEBUG == 1
        USB.println(F("data string overflow."));
      #endif
      return 1;
    }
    
    if(pairInd < numPairs-1)                  //  if it isn't the last keyvalue, separate with a comma
    {
      dataString[len] = ',';
      len++;
    }
  }
  
  if(len < DATASIZE-1)                        //  if doing so won't overflow the string
  {
  dataString[len] = ';';                      //  add a semicolon to delineate successive measurement cycles
  len++;
  dataString[len] = 0;                        //  terminate the string
  len++;
  }
  else                                        //  too many characters to fit in dataString
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("data string too long."));
    #endif
    return 1;
  }

  // print out the dataString to verify its format
  #if GLACIERPROBE_DEBUG == 1
    for(int i = 0; i<sizeof(dataString); i++){
      USB.print(dataString[i]);
    }
  #endif


  //  Step 2: 
  //  initialize the SD card
  if(!SD.ON())                                //  if the SD card fails to turn on
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Failed to init SD"));
    #endif
    return 2;
  }

  #if GLACIERPROBE_DEBUG == 1
    USB.println(filename);
  #endif
  
  //  Step 3:
  //  Check if file exists, if not then create it, open the file
  if(SD.isFile(filename)==-1)
  {
    SD.create(filename);
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("File created"));
    #endif
  }

  //  Step 4:
  //  Append dataString to the end of the file on a new line.
  if(SD.appendln(filename, dataString))
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("SD append success"));
    #endif
  }
  else
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("SD append failure"));
      for(int i=0; i<sizeof(SD.buffer); i++)            // print out the error message stored in the SD object
      {
        USB.print(SD.buffer[i]);
      }
    #endif
    
    SD.OFF();
    return 3;
  }

  //  Step 5:
  //  Close the file and turn off the SD card
  SdFile* currDir = &SD.currentDir;                     //  the directory to the file is stored as an SdFile object
  uint8_t error = SD.closeFile(currDir);                //  closeFile takes the SdFile object, not the character array
  
  
  if(error != 1)
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Failed to close directory"));
    #endif
    SD.OFF();
    return 4;
  }
  
  SD.OFF();                                             //  turn of the SD while we don't use it

  #if GLACIERPROBE_DEBUG == 1
    USB.println(F("SD write completed, directory closed"));
  #endif
  
  return 0;
}

/*
updateTimes()
Updates the data interval dynamically, so if the interval is supposed to be 60 seconds but
it has been 62 seconds since the last cycle then it will reduce the interval to 58. Also
updates the seconds variable. Also checks the date to see if it has changed, and if so returns
1 to indicate that the file should transmitted and a new file should be created.

Parameters:
- uint16_t seconds: the timestamp in seconds
- char wtoStr[12]: character array representing the wakeTimeOffset for setting the RTC alarm.
Returns:
- 0 if the date is the same
- 1 if the date changed
 */
 
bool updateTimes( char* seconds,
                  char wtoStr [12]){
  
  uint32_t lastTime = (uint32_t) RTC.hour * 3600 + RTC.minute*60 + RTC.second;
  
  RTC.getTime();                                                  //  update the time variables
  //get the new time
  uint32_t currentTime = (uint32_t) RTC.hour*3600 + RTC.minute*60 + RTC.second;
  sprintf(seconds, "%lu", currentTime);  //  5 chars max, update the seconds character array

  //update the wakeTime interval
  timestamp_t wto;
  RTC.breakTimeOffset(DATA_INTERVAL, &wto);
  
  sprintf_P(wtoStr, PSTR("%.2u:%.2u:%.2u:%.2u"), wto.date, wto.hour, wto.minute, wto.second);
  #if GLACIERPROBE_DEBUG == 1
    USB.printf("lastDATE: %u\tcurrDATE: %u\n",lastDate, RTC.date);
  #endif
  
  if(RTC.date != lastDate){ 
    lastDate = RTC.date;             
    return 1;                            //  the day changed
                
  }

  return 0;                              // the day didn't change
  
}

/*
unsentFile()
Appends the current SD_filename to the list of files that have not been sent to the FTP server.
This file list can later be checked for files that have not been sent due to errors such as a
loss of connection and the device can retry sending them.

Returns:
- 0 if the file was successfully added to the list
- 1 if the SD fails to initialize
- 2 if the SD couldn't append the filename for some reason
 */

uint8_t appendUnsentFile()
{
  
  if(!SD.ON())
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Failed to init SD"));
    #endif
    return 1;                           //  SD failed to initialize
  }

  char fList [20] = { 0 };                      //  name of the unsent file list
  strcpy_P(fList, UNSENT_FILES_NAME );
  
  if(SD.isFile(fList)==-1)              //  if the file doesn't exist, create it
  {
    SD.create(fList);
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Filelist created"));
    #endif
  }

  if( SD.appendln(fList, SD_filename) ) //  if the filename is successfully appended
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("file added to list of unsent files"));
    #endif
    SD.OFF();
    return 0;
  }

  #if GLACIERPROBE_DEBUG == 1           //  otherwise something went wrong
    USB.println(F("ERROR: Could not append filename to list of unsent files."));
  #endif
  SD.OFF();
  return 2;
  
}

/*
checkUnsentFiles()
Searches through the list of unsent files. Filenames that start with '*' have been marked
as sent. The search stops if it takes longer than a minute to send any unsent files, if the
last file in the list is the current day's file, if the SD card reads an empty line, or if 
for some reason the file directory buffer would overflow if the name is added (probably a
corrupt file).

Returns:
- 0 if all files have been sent to the FTP server
- 1 if the SD fails to initialize
- 2 if the directory buffer would overflow
- 3 if the operations took longer than a minute
- 4 if the current day's file hasn't been added
 */

uint8_t checkUnsentFiles()
{

  if(!SD.ON())
  {
    return 1;                           //  SD failed to initialize
  }

  char fList [20] = {0};                      //  name of the list of files
  memset(fList, 0, sizeof(fList));
  strcpy_P(fList, UNSENT_FILES_NAME );
  
  #if GLACIERPROBE_DEBUG == 1
    USB.print(F("File List: "));
    USB.println(fList);
  #endif
  
  if(SD.isFile(fList)==-1)              //  if the file list doesn't exist, create it
  {
    SD.create(fList);
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("File created"));
    #endif
  }
  #if GLACIERPROBE_DEBUG == 1
    USB.println(F("scanning list..."));
  #endif
    
  uint16_t i = 0;                       //  index representing line number in the SD file
  uint32_t start = millis();
  while ( millis() - start < 60000 )    //  time out if the rest of the operations take longer than a minute
  {
    SD.catln(fList, i, 1);              //  read in a line from the file and store it in the SD buffer
    
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("SD Buffer: "));
      USB.println(SD.buffer);
    #endif
    
    if( strncmp( SD.buffer, SD_filename, 8 ) == 0 ) //  if the filename is the same as the current day's file, finish
    {
      SD.OFF();
      return 0;
    }

    if( strlen( SD.buffer ) == 0 )       // if the SD reads in a blank line, finish
    {
      SD.OFF();
      return 4;
    }

    if( sizeof(FTP_DIR) + strlen(SD.buffer) >= 40 ) //  if adding the filename to the FTP directory would overflow,
    {                                               //  an error occurred (such as corrupt data)
      SD.OFF();
      return 2;
    }

    if( SD.buffer[0] != '*')             // if the file hasn't been sent previously
    {
      //  get the FTP directory using the filename of the first unsent file
      char FTP_dir [40] = { 0 };
      strcpy(FTP_dir, FTP_DIR);
      strncpy(FTP_dir + sizeof(FTP_DIR) - 1, SD.buffer, 13);

      #if GLACIERPROBE_DEBUG == 1
        USB.print(F("Directory to upload to server: "));
        USB.println(FTP_dir);
      #endif

      char sd_fname [13];
      strncpy(sd_fname, SD.buffer, 12);
      sd_fname[12] = 0;
      #if GLACIERPROBE_DEBUG == 1
        USB.print(F("File to upload:"));
        USB.println(sd_fname);
        USB.print(F("Length of fname: "));
        USB.println(strlen(sd_fname));
      #endif
      //  attempt to send the file to the FTP server
      if ( comms.postFTP(FTP_SERVER, FTP_PORT, FTP_USER, FTP_PASS, sd_fname, FTP_dir) == 1 )
      {
        #if GLACIERPROBE_DEBUG == 1
          USB.println(F("Upload complete, marking file as sent."));
        #endif
        markSentFile(sd_fname);  //  mark file as sent
      }
    }

    i++;                              //  increment to the next line
  }
  
  SD.OFF();
  return 3;                           //  timeout condition was met
}

/*
markSentFile()
Marks the specified file as sent in the list of unsent files.

Parameters:
- char* fname: name of the file to mark as sent

Returns:
- 0 if all files have been sent to the FTP server
- 1 if the SD fails to initialize
- 2 if there was an error writing to the SD
 */

uint8_t markSentFile(char* fname)
{

  // get the name of the list of sent files
  char fList [20] = {0};
  strcpy_P(fList, UNSENT_FILES_NAME );

  //  get the index of the start of the filename to mark
  int32_t idx = SD.indexOf(fList, fname, 0);

  #if GLACIERPROBE_DEBUG == 1
    USB.println(F("Found filename in index: "));
    USB.printf("%lu\n",idx);
  #endif
  char* c = "*"; 
  if( SD.writeSD(fList, c, idx) == 0 )  //  if there was an error marking it
  {
    #if GLACIERPROBE_DEBUG == 1
      USB.println(F("Failed to mark file"));
    #endif
    return 2;
  }
  return 0; // if the file was successfully marked as sent
}

#endif












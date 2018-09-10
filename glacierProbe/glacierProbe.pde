#include "header.h"
           
my4G comms("gprs.swisscom.ch", "gprs", "gprs");

uint8_t battery = BL_HIGH;
bool BL_changed = false;

const char SMS_CMD_KEY_TIME [] PROGMEM = "Time";
const char SMS_CMD_KEY_SIGNAL [] PROGMEM = "RSSI";
const char SMS_CMD_KEY_BATTERY [] PROGMEM = "Battery";
const char SMS_CMD_KEY_FAILED []  PROGMEM = "CMD";
const char SMS_CMD_VAL_FAILED []  PROGMEM = "FAILED";

const char* const SMS_CMD_KEYS[] PROGMEM =
{
  SMS_CMD_KEY_TIME,
  SMS_CMD_KEY_SIGNAL,
  SMS_CMD_KEY_BATTERY,
  SMS_CMD_KEY_FAILED,
  SMS_CMD_VAL_FAILED
};

keyvalue currData[] = {keyvalue("temperature"),   //  BME
                       keyvalue("humidity"),      //  BME
                       keyvalue("pressure"),      //  BME
                       keyvalue("sonic"),         //  SONIC
                       keyvalue("wetness"),       //  PHYTOS
                       keyvalue("solar"),         //  SOLAR
                       keyvalue("ubar"),          //  DS2
                       keyvalue("vbar"),          //  DS2
                       keyvalue("gust"),          //  DS2
                       keyvalue("wSpeed"),        //  DS2
                       keyvalue("wDirect"),       //  DS2
                       keyvalue("ds2Temp"),       //  DS2
                       keyvalue("seconds")        //  timestamp
};      



void setup(){
  USB.ON();
  RTC.ON();
  RTC.getTime();
  lastDate = RTC.date;
  setFileNames(SD_filename, sizeof(SD_filename), FTP_filename, sizeof(FTP_filename));
}

/*
 * Loop Flow:
 * 1. If the battery power allows, check for unsent files in the SD card's file "fList.txt"
 * 2. If there are unsent files, send them to the FTP server.
 * 3. If the current date doesn't have its file in "fList.txt", append its name to the list
 * 4. 
 */

void loop(){
  updateBatteryLevel();

  switch( battery )
  {
    case BL_HIGH:
      execute_BL_HIGH();
      break;
    case BL_MEDIUM:
      execute_BL_MEDIUM();
      break;
    case BL_LOW:
      execute_BL_LOW();
      break;
    case BL_CRITICAL:
      execute_BL_CRITICAL();
      break;
  }
  
  USB.ON();
  RTC.ON();
  
}

void updateBatteryLevel()
{
  uint8_t b = PWR.getBatteryLevel();
  
  if(b > 80)
  {
    if( battery != BL_HIGH)
    {
      BL_changed = true;
      battery = BL_HIGH;
    }
    else { BL_changed = false; }  
    return;
  }

  if(b > 40)
  {
    if( battery != BL_MEDIUM)
    {
      BL_changed = true;
      battery = BL_MEDIUM;
    }
    else { BL_changed = false; }
    
    return;
  }

  if(b > 15)
  {
    if( battery != BL_LOW)
    {
      BL_changed = true;
      battery = BL_LOW;
    }
    else { BL_changed = false; }
     
    return;
  }
  if( battery != BL_CRITICAL)
    {
      BL_changed = true;\
      battery = BL_CRITICAL;
    }
  else { BL_changed = false; }
  
  return;
}



/*
checkSMSCommand
Checks unread messages for any commands in a predefined list in the class, then parses it into a value representing
the index in the command table and switches the function based on the value. The valid commands are:

"*DATA!" - dweet the current sensor data
"*TIME!" - dweet the current time of day
"*SIGNAL!" - dweet the RSSI (signal strength)
"*BATTERY!"  - dweet the battery percentage
"*RESET!"  - reboot the device
"*SET TIME!HH:MM:SS" - change the RTC's time of day to the specified time


Parameters:
- none, automatically fetches the cmd value by calling my4G.readSMSCommand()
Returns:
- 0 if a command was valid and carried through
- 1 if no valid commands were found
- 2 if the message got corrupted for the SET TIME command
 */

uint8_t runCommand(int8_t cmd)
{

  //  checks unread SMS messages for a known command, then performs some actions based on which
  //  command it was.


  //  get the index of the SMS command that was received, then switch to different functions based
  //  on the result

  //  create a char array to represent the exact date and time to pass to the RC
  char newTime [21] = {0};
  uint8_t i = 0;

  char c;

  //  most of the actions require dweeting a response, so just set up the name here
  char name [15] = {0};
  
  strncpy_P(name, DEVICE_NAME, sizeof(name));
  keyvalue kv_buff; //  just a blank keyvalue object to store different data depending on the case

  switch(cmd)
  { //  we'll switch to different functions based on the command index that was sent
    //  all command indices are defined in my4G.h, which is included directly.
    
    case SMS_CMD_DATA:  //  commands are defined in my4G.h
    
      //  the user requested to view the current sensor data (to verify every sensor is working probably)

      //  turn on 4G, send a dweet of the current data, and then turn it off and return to the main program.
      comms.ON();
      comms.sendDweet( DWEET_PORT,  //  80 for http without encription
                      name,         //  device name
                      sizeof(name), //  length of device name
                      currData,     //  user requested current data, which we already have a kv array for
                      NUM_KEYVALS); //  number of different measurements in kv array, defined in header
      comms.OFF();
      
      return 0;
      break;
    
    case SMS_CMD_TIME:
      //  the user requested to view the RTC's current time of day.

      //  set up a keyvalue representing the time.
      memset(kv_buff.key, 0, kv_buff.KEYVAL_STRING_SIZE);
      strcpy_P(kv_buff.key, SMS_CMD_KEYS[0]);
      memset(kv_buff.val, 0, kv_buff.KEYVAL_STRING_SIZE);
      
      snprintf( kv_buff.val, 
                kv_buff.KEYVAL_STRING_SIZE,
                "%.2u:%.2u:%.2u", RTC.hour, RTC.minute, RTC.second);


      //  turn on 4G, send the dweet of a single keyvalue, turn it off and return to the main program.
      comms.ON();

      //  sendDweet expects a kv pointer but we only have one object, not an array, so we pass its
      //  address instead and pretend it's an array of length 1.
      comms.sendDweet( DWEET_PORT,
                      name,
                      sizeof(name),
                      &kv_buff,     //  pass by reference
                      1);           //  it's just a single object, so length = 1
      comms.OFF();
      
      return 0;
      break;

    case SMS_CMD_SIGNAL:
      //  the user requested to view the strength of the GPRS signal

      //  set up the keyvalue representing the RSSI signal strength
      memset(kv_buff.key, 0, kv_buff.KEYVAL_STRING_SIZE);
      strcpy_P(kv_buff.key, SMS_CMD_KEYS[1]);
      memset(kv_buff.val, 0, kv_buff.KEYVAL_STRING_SIZE);
      comms.ON();
      comms.checkDataConnection(15000);
      comms.getRSSI();
      sprintf(kv_buff.val, "%d", comms._rssi);

      //  turn 4G on, send the single keyvalue dweet, and turn it off and return to the main program.
      comms.sendDweet( DWEET_PORT,
                      name,
                      sizeof(name),
                      &kv_buff,
                      1);     
      comms.OFF();
      return 0;
      break;

    case SMS_CMD_BATTERY:
      //  the user requested to view the battery percentage

      //  set up the keyvalue representing the battery level as a percentage
      memset(kv_buff.key, 0, kv_buff.KEYVAL_STRING_SIZE);
      strcpy_P(kv_buff.key, SMS_CMD_KEYS[2]);
      memset(kv_buff.val, 0, kv_buff.KEYVAL_STRING_SIZE);
      
      snprintf(kv_buff.val,kv_buff.KEYVAL_STRING_SIZE, "%u", PWR.getBatteryLevel());

      //  turn 4G on, send the dweet, turn it off and return to the main program.
      comms.ON();
      comms.sendDweet( DWEET_PORT,
                       name,
                       sizeof(name),
                       &kv_buff,
                       1);
      comms.OFF();
      return 0;
      break;
      
    case SMS_CMD_RESET:
      //  the user requested to reboot the WaspMote.

      //reboot. The return probably isn't necessary but it's included for consistency.
      runCommand(SMS_CMD_DATA);
      PWR.reboot();
      return 0;
      break;

    case SMS_CMD_SETTIME:
      //  The user is changing the time of day of the RTC.

      //  20 characters representing "year:month:date:weekday:hour:minute:second", two char each plus ':' 
      //  between them, plus null terminator
        
      
      c = comms._buffer[i];  //  read in a character
      
      while(c != '!' && c != 0)
      {
        //  skip to the start of the value of the command
        c = comms._buffer[i];
        i++;
      }

      if( strlen( (char*) (comms._buffer+i) ) >= 21 &&
          *(comms._buffer+i+2) == ':' &&
          *(comms._buffer+i+17) == '"')//  if we aren't at the end of the message (would be a weird error if we got this far)
      {

        //  set up the first half of the string representing the complete date
        //  we need to append 13 characters because it adds a null terminator
        strncpy(newTime, (char*) (comms._buffer + i), 9);
        i += 12;
        snprintf(newTime+9, 4, "%.1u:", RTC.day);
        //  read in the expected "HH:MM:SS" after the '!' symbol and append it to newTime[], overwriting the previous null terminator
        strncpy(newTime + 11, (char*) (comms._buffer + i), 8);  //  we are expecting 8 bytes representing the hour, minute, second
        newTime[20] = 0;                            //  terminate the char array to make it a classic string

        USB.printf("NewTime: %s", newTime);
        //RTC.setTime(newTime); //  send the new time
        RTC.setTime( newTime );
        runCommand(SMS_CMD_TIME);
        return 0;
      }

      //  "CMD=FAILED"
      memset(kv_buff.key,0,kv_buff.KEYVAL_STRING_SIZE);
      memset(kv_buff.val,0,kv_buff.KEYVAL_STRING_SIZE);
      strcpy_P(kv_buff.key, SMS_CMD_KEY_FAILED);
      strcpy_P(kv_buff.val, SMS_CMD_VAL_FAILED);

      comms.ON();
      comms.sendDweet( DWEET_PORT,
                      name,
                      sizeof(name),
                      &kv_buff,
                      1);     
      comms.OFF();
      
      return 2; //  this is here in case, for some reason, the message gets corrupted and things are not in the expected order
      break;
      
    default: // if the user didn't enter a real command then just don't do anything, return 1
      return 1;
      break;
  }
}

void execute_BL_HIGH()
{
  char dname [20] = {0};  //  device name buffer
  strncpy_P(dname, DEVICE_NAME, sizeof(dname));
  //  if battery level changed:
  if( BL_changed == true )
  {
    keyvalue batt = ("BATTERY");
    strcpy(batt.val, "HIGH");
    comms.sendDweet( 80, dname, strlen(dname), &batt, 1 );
  }
  //  check the unsent files list for files that need sending
  uint8_t result = checkUnsentFiles();
  if( result == 4 ) //  if the current date is not included, append it to the list
  {
    appendUnsentFile();
  }

  //  prepare the timestamp and waketime offset
  char wtoStr [12] = {0};                 //  wake-time offset
  bool newFile = updateTimes(currData[KV_SECONDS].val, wtoStr); //  get the seconds and update wake time offset

  //  get sensor data
  readAllSensors(currData);

  //  set the file names based on the current date and/or time
  setFileNames( SD_filename,            //  update the name of the SD file based on current time
                sizeof(SD_filename),    //  max length of the filename
                FTP_filename,           //  update the directory and file of the FTP server
                sizeof(FTP_filename));  //  max length of the ftp server director

  //  write the data to the SD file
  writeDataSet(currData, NUM_KEYVALS, SD_filename); //  write the data set to the SD file.

  //  prep to get dweet info
  comms.ON();
  

  //  response from checking dweet server for user command
  int8_t ans = comms.receiveDweetCommand(dname);  //  index for command received, or error code
  comms.OFF();
  ans = runCommand(ans);  //  execute the command

  #if GLACIERPROBE_DEBUG == 1
    USB.printf("Command Received: %d\n",ans);
  #endif

  PWR.deepSleep(wtoStr, RTC_OFFSET, RTC_ALM1_MODE4);
}

void execute_BL_MEDIUM()
{
  char dname [20] = {0};  //  device name buffer
  strncpy_P(dname, DEVICE_NAME, sizeof(dname));
  
  if( BL_changed == true )
  {
    keyvalue batt = ("BATTERY");
    strcpy(batt.val, "MEDIUM");
    comms.sendDweet( 80, dname, strlen(dname), &batt, 1 );
  }
  //  ********************************************
  //  AFTER THIS POINT THIS IS THE SAME AS BL_HIGH
  //  ********************************************

  //  check the unsent files list for files that need sending
  uint8_t result = checkUnsentFiles();
  if( result == 4 ) //  if the current date is not included, append it to the list
  {
    appendUnsentFile();
  }

  //  prepare the timestamp and waketime offset
  char wtoStr [12] = {0};                 //  wake-time offset
  bool newFile = updateTimes(currData[KV_SECONDS].val, wtoStr); //  get the seconds and update wake time offset

  //  get sensor data
  readAllSensors(currData);

  //  set the file names based on the current date and/or time
  setFileNames( SD_filename,            //  update the name of the SD file based on current time
                sizeof(SD_filename),    //  max length of the filename
                FTP_filename,           //  update the directory and file of the FTP server
                sizeof(FTP_filename));  //  max length of the ftp server director

  //  write the data to the SD file
  writeDataSet(currData, NUM_KEYVALS, SD_filename); //  write the data set to the SD file.

  //  prep to get dweet info
  comms.ON();

  //  response from checking dweet server for user command
  int8_t ans = comms.receiveDweetCommand(dname);  //  index for command received, or error code
  comms.OFF();
  ans = runCommand(ans);  //  execute the command

  #if GLACIERPROBE_DEBUG == 1
    USB.printf("Command Received: %d\n",ans);
  #endif

  PWR.deepSleep(wtoStr, RTC_OFFSET, RTC_ALM1_MODE4);
  
}

void execute_BL_LOW()
{
  char dname [20] = {0};  //  device name buffer
  strncpy_P(dname, DEVICE_NAME, sizeof(dname));
  
  if( BL_changed == true )
  {
    keyvalue batt = ("BATTERY");
    strcpy(batt.val, "LOW");
    comms.sendDweet( 80, dname, strlen(dname), &batt, 1 );
  }

  //  check the unsent files list for files that need sending
  //  won't send them if battery level is LOW or CRITICAL
  uint8_t result = checkUnsentFiles();
  if( result == 4 ) //  if the current date is not included, append it to the list
  {
    appendUnsentFile();
  }

  //  prepare the timestamp and waketime offset
  char wtoStr [12] = {0};                 //  wake-time offset
  bool newFile = updateTimes(currData[KV_SECONDS].val, wtoStr); //  get the seconds and update wake time offset

  //  get sensor data
  readAllSensors(currData);

  //  set the file names based on the current date and/or time
  setFileNames( SD_filename,            //  update the name of the SD file based on current time
                sizeof(SD_filename),    //  max length of the filename
                FTP_filename,           //  update the directory and file of the FTP server
                sizeof(FTP_filename));  //  max length of the ftp server director

  //  write the data to the SD file
  writeDataSet(currData, NUM_KEYVALS, SD_filename); //  write the data set to the SD file.

  //  DON'T communicate over 4G.

  PWR.deepSleep(wtoStr, RTC_OFFSET, RTC_ALM1_MODE4);
}

void execute_BL_CRITICAL()
{
  char dname [20];
  strcpy_P( dname, DEVICE_NAME );
  
  if( BL_changed == true )
  {
    keyvalue batt = ("BATTERY");
    strcpy(batt.val, "CRITICAL");
    comms.sendDweet( 80, dname, strlen(dname), &batt, 1 );
  }

  //  check the unsent files list for files that need sending
  uint8_t result = checkUnsentFiles();
  if( result == 4 ) //  if the current date is not included, append it to the list
  {
    appendUnsentFile();
  }

  //  prepare the timestamp and waketime offset
  char wtoStr [12] = {0};                                       //  wake-time offset
  bool newFile = updateTimes(currData[KV_SECONDS].val, wtoStr); //  get the seconds and update wake time offset

  //  get sensor data
  readAllSensors(currData);

  //  set the file names based on the current date and/or time
  setFileNames( SD_filename,            //  update the name of the SD file based on current time
                sizeof(SD_filename),    //  max length of the filename
                FTP_filename,           //  update the directory and file of the FTP server
                sizeof(FTP_filename));  //  max length of the ftp server director

  //  write the data to the SD file
  writeDataSet(currData, NUM_KEYVALS, SD_filename); //  write the data set to the SD file.

  //  DON'T communicate over 4G.

  PWR.deepSleep(wtoStr, RTC_OFFSET, RTC_ALM1_MODE4);
}





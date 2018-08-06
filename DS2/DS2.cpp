
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

}

uint8_t DS2::ON()
{
	char message[70];

	if(AgricultureXtr.redefinedSocket == 1)
	{
		//"WARNING: Redefinition of sensor socket detected"
		strcpy_P(message, (char*)pgm_read_word(&(table_agrxtr[6]))); 
		PRINTLN_AGR_XTR(message);
	}

	if((_socket == AGR_XTR_SOCKET_E) || (_socket == AGR_XTR_SOCKET_F))
	{
		//"WARNING - The following sensor can not work in the defined socket:"
		strcpy_P(message, (char*)pgm_read_word(&(table_agrxtr[7]))); 
		PRINT_AGR_XTR(message);
		#if DS2_DEBUG == 1
			USB.println(F("SF-421"));
		#endif

		return 0;
	}

	super::ON();		//SDI12 needs both 3v3 and 5v
	set12v(_12V_ON);

	delay(300);			//same delay after powering sensor as Apogee SF421, may not be necessary

	setMux();

	//	basically a copy of the isSensor code, but prints out the response instead of checking if the sensor is supported
	strcpy_P(command, (char*)pgm_read_word( &( table_agrxtr[5] ) ) );
	sdi12.sendCommand(command, strlen(command));

	sdi12.readCommandAnswer(33, LISTEN_TIME);

	delay(30);

	if(sdi12.available() >= 20)
	{
		#if DS2_DEBUG == 1
			USB.print("DS2 Response: ");
			while(sdi12.available())
			{
				USB.print(sdi12.read());
			}
			USB.println();
		#endif
		return 0;
	}

	#if DS2_DEBUG == 1
		USB.println("DS2 Unresponsive.");
	#endif

	sdi12.setState(DISABLED);
	digitalWrite(MUX_EN, HIGH);
	this->OFF();

	return 1;
}

void DS2::OFF()
{
	set12v(_12V_OFF);
	super::OFF();
}

uint8_t DS2::read()
{
	memset(ubar, 0, strSize);
	memset(vbar, 0, strSize);
	memset(gust, 0, strSize);
	memset(windSpeed, 0, strSize);
	memset(windDirection, 0, strSize);
	memset(temperature, 0, strSize);
	memset(checkSum, 0, 10);

	char aux[4] = {};						//	consistent with Xtr::read(), aux is the command to be sent

	if(super.startSensor() == 0)
	{
	//	if the sensor is unresponsive, disable SDI-12 comms and then return from the function
		sdi12.setState(DISABLED);
		digitalWrite(MUX_EN, HIGH);
		return 1;
	}
	uint8_t time = atoi(timeToNextMeasure);
	delay(time*1000);

	strcpy(aux, (char*)pgm_read_word( &( table_agrxtr[2] ) ) );
	snprintf(command, sizeof(command), "%c%s", address, aux);

	sdi12.sendCommand( command, strlen(command) );
	sdi12.readCommandAnswer( 30, LISTEN_TIME );

	if( sdi12.available() )
	{
		uint8_t counter = 0;
		uint8_t charIdx = 0;

		while(	sdi12.available() &&		//	there are bytes left in the message
				counter < 3 &&				//	we haven't exceeded the number of measurements
				charIdx < strSize - 1)		//	we haven't overflowed our measurements
		{
			char c = sdi12.read();
			
			if ( c == '+' || c == '-' )
			{
				counter++;
				charIdx = 0;
			}
			switch(c)
			{
				case 1:
					windSpeed[charIdx] = c;
					charIdx++;
					windSpeed[charIdx] = 0;
					break;
				case 2:
					windDirection[charIdx] = c;
					charIdx++;
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

		#if DS2_DEBUG == 1
			USB.printf("Measurements:\nWS: %s\nWD: %s\nTemp: %s\n", windSpeed, windDirection, temperature);
		#endif
	}
	else 
	{
		#if DS2_DEBUG == 1
			USB.println(F("No response to 'aD0!'"));
		#endif
		return 1;
	}

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "%c%s", address, "D3!")

	sdi12.sendCommand(command, strlen(command));
	sdi12.readCommandAnswer(5,LISTEN_TIME);

	if( sdi12.available() )
	{
		uint8_t counter = 0;
		uint8_t charIdx = 0;

		while(	sdi12.available() &&		//	there are bytes left in the message
				counter < 3 &&				//	we haven't exceeded the number of measurements
				charIdx < strSize - 1)		//	we haven't overflowed our measurements
		{
			char c = sdi12.read();
			
			if ( c == '+' || c == '-' )
			{
				counter++;
				charIdx = 0;
			}
			if( c != 'CR')
			{
				switch(c)
				{
					case 1:
						ubar[charIdx] = c;
						charIdx++;
						ubar[charIdx] = 0;
						break;
					case 2:
						vbar[charIdx] = c;
						charIdx++;
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
			}
		}

		if ( sdi12.available())
		{
			checksum = sdi12.read();
		}

		if(checksum != calcChecksum())
		{
			#if DS2_DEBUG == 1
				USB.println(F("Checksum Failed."));
			#endif
			return 3;
		}

		#if DS2_DEBUG == 1
			USB.printf("Measurements:\nubar: %s\nvbar: %s\ngust: %s\nCS: %c\n",	ubar, vbar, gust, checkSum);
			USB.println(F("All measurements successful."));
		#endif
	}
	else
	{
		#if DS2_DEBUG == 1
			USB.println(F("No response to 'aD3!'."));
		#endif
		return 2;
	}

	return 0;

}

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

char DS2::calcChecksum()
{
	int crc = 0;
	for( int i=0; i<strSize; i++ )
	{
		crc += ubar[i];
		crc += vbar[i];
		crc += gust[i];
	}

	crc = (crc % 64) + 32;

	return crc;
}








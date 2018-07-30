#ifndef STRUCTURES_H
#define STRUCTURES_H
/*
structures.h
Author: Mitch Nelke
Date: July 27, 2018
*/


// 	measurements are stored as key value pairs in the keyvalue object. Both are character arrays
//	10 bytes long so they are uniform.
struct keyvalue {
	const static uint8_t KEYVAL_STRING_SIZE = 16;
	char key [KEYVAL_STRING_SIZE];	//name
	char val [KEYVAL_STRING_SIZE];	//data

	keyvalue(char* k){
		if(sizeof(k)<KEYVAL_STRING_SIZE)
		{
			strcpy(key,k);
			char v [] = "";
			strcpy(val,v);
		}
		else keyvalue();

	}
	keyvalue(){
		char k [] = "";
		char v [] = "";
		strcpy(key, k);
		strcpy(val, k);
	}

};

#endif

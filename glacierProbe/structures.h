#pragma once

/*
structures.h
Author: Mitch Nelke
Date: July 27, 2018
*/


// 	measurements are stored as key value pairs in the keyvalue object. Both are character arrays
//	10 bytes long so they are uniform.
struct keyvalue {
	const static uint8_t KEYVAL_STRING_SIZE = 10;
	char key [KEYVAL_STRING_SIZE];	//name
	char val [KEYVAL_STRING_SIZE];	//data
};

keyvalue* currData [6];
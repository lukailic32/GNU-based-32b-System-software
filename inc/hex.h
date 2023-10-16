#ifndef HEX_H
#define HEX_H

#include <string>
#include <list>
#include <map>
#include <iostream>

using namespace std;

string translateToHex(unsigned int arg);

string translateToHexLittleEndian(unsigned int arg);

string translateWithOstring(unsigned int arg);

string translateToHexExtended(unsigned int arg);

string wordToHex(uint8_t* word);

int hexadecimalToDecimal(string hexVal);

#endif
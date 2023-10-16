#include "../inc/hex.h"
#include <sstream>
#include <iomanip>


map<uint8_t, string> hexTranslator = {
  {0, "0"},
  {1, "1"},
  {2, "2"},
  {3, "3"},
  {4, "4"},
  {5, "5"},
  {6, "6"},
  {7, "7"},
  {8, "8"},
  {9, "9"},
  {10, "A"},
  {11, "B"},
  {12, "C"},
  {13, "D"},
  {14, "E"},
  {15, "F"}
};

string translateToHex(unsigned int arg){
  int higher = arg >> 4;
  int lower = arg & 0xF;
  return hexTranslator[higher] + hexTranslator[lower];
}

string translateToHexLittleEndian(unsigned int arg){
  int first = arg & 0xFF;
  string firstString = translateToHex(first);
  int second = (arg >> 8) & 0xFF;
  string secondString = translateToHex(second);
  int third = (arg >> 16) & 0xFF;
  string thirdString = translateToHex(third);
  int fourth = (arg >> 24) & 0xFF;
  string fourthString = translateToHex(fourth);
  return firstString + secondString + thirdString + fourthString; 
}

string translateWithOstring(unsigned int arg) {
    ostringstream oss;

    for (int i = 0; i < sizeof(unsigned int); ++i) {
        int byteValue = (arg >> (i * 8)) & 0xFF;
        oss << std::hex << std::setw(2) << std::setfill('0') << byteValue;
    }

    return oss.str();
}

string translateToHexExtended(unsigned int arg) {
  int first = (arg >> 24) & 0xFF;
  string firstString = translateToHex(first);
  int second = (arg >> 16) & 0xFF;
  string secondString = translateToHex(second);
  int third = (arg >> 8) & 0xFF;
  string thirdString = translateToHex(third);
  int fourth = (arg) & 0xFF;
  string fourthString = translateToHex(fourth);
  return firstString + secondString + thirdString + fourthString; 
}

string wordToHex(uint8_t* word){
  int first = word[0];
  string firstString = translateToHex(first);
  int second = word[1];
  string secondString = translateToHex(second);
  int third = word[2];
  string thirdString = translateToHex(third);
  int fourth = word[3];
  string fourthString = translateToHex(fourth);
  return firstString + secondString + thirdString + fourthString; 
}

int hexadecimalToDecimal(string hexVal) {
  int len = hexVal.size();
  int base = 1;
  int dec_val = 0;

  for (int i = len - 1; i >= 0; i--) {
    if (hexVal[i] >= '0' && hexVal[i] <= '9') {
      dec_val += (int(hexVal[i]) - 48) * base;
      base = base * 16;
    } else if (hexVal[i] >= 'A' && hexVal[i] <= 'F') {
        dec_val += (int(hexVal[i]) - 55) * base;
        base = base * 16;
    }
  }
  return dec_val;
}
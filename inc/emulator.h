#ifndef EMULATOR_H
#define EMULATOR_H
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include "hex.h"

using namespace std;


class Emulator {
private:
  string input_hex_file;
  //proveri
  map<unsigned int, unsigned int> memory;
  ifstream input_file;

  void pushPC();
  void decSP();
  void pushStatus();
  void incPC();
  void maskInterrupts();
  void readRegs(unsigned int);

  void writeOutput();

public:

  void emulate();
  void readHexFile();

  Emulator(string);

};

#endif
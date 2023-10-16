#ifndef LINKER_H
#define LINKER_H


#include "./hex.h"
#include <fstream>
#include <queue>
#include <vector>

using namespace std;

struct symbolData {
  string type;
  unsigned int address;
  bool defined;
  string name;
};

struct sectionData {
  int id;
  int size;
  string sectionName;
};

struct relocationEntry {
  string name;
  unsigned int address;
  int secId;
};

struct sectionWithAddress {
  string section;
  unsigned int address;
};

struct orderElement {
  string fileName;
  string sectionName;
  int sectionIndex;
  unsigned int startAddr;
  unsigned int endAddr;
} ;

class Linker {
private:

  void sortSectionsInOrder();
  bool proccessLine(string);

  void proccessCode(string);
  void proccessReloc(string);
  void proccessSymbol(string);
  void proccessSection(string);

  void adjustSymbolTable();
  void adjustRelocationTable();
  void getDefinedSymbols();
  void resolveSymbols();
  void verifyAllDefined();
  void createOutput();
  void writeOutputFile();

  void checkForSectionsWithSameNameDiffrentFile(string file, string section);
  bool sectionFromFileNotInList(string file, string section);

public:

  void linkerInitialize();
  vector<string> fileNamesInOrder;
  vector<sectionWithAddress> sectionsInOrder;
  string hex_output_file;

  void generateOutput();
  void generateMemoryAndConnectTables();


  Linker(string outputFileName, vector<sectionWithAddress> &secInOrd, vector<string> &filNamInOrd);
  bool linc();
  void orderSections();

};


#endif
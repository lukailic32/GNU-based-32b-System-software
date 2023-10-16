#ifndef _ASSEMBLER_HPP__
#define _ASSEMBLER_HPP__

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <list>


using namespace std;

struct EntrySymbolTable
{
    int id_symbol;   // sequence number of symbol
    int value;       // this is offset within section
    int size;
    bool isSection;
    bool is_local;   // is symbol local or global
    bool is_defined; // is symbol defined (if it is local it has to be)
    bool is_extern;  // is symbol extern (then it is undefined)
    string section;  // section where this symbol is defined
    string name;     // symbol name
};

struct EntryRealocationTable
{
    bool is_data;        // is this relocation of data or of instruction
    string section_name; // where to relocate, current section!
    int offset;          // which byte is the start byte for relocation
    string type;         // type of relocation
    string symbol_name;  // which symbol is relocated ! (for local symbol it is the section where it is defined)
    int addend;          // unused, leaved here for eventually
};

struct EntrySectionTable
{
    int id_section;      // sequence number of section
    string section_name; // name of section
    int size;            // size of section
    int beginnningAddress; // ovo po defaultu stavljam na 0, pa ce linker daazurira
    vector<int> offsets; // beginning of data -> location_counter
    vector<uint8_t> data;   // data at offset in vector offsets
    vector<EntryRealocationTable> tableOfRealocations;
};

struct EntryLiteralPool {
  int address;
  int value;
};

class Assembler{
private:
  string input_file_name;
  string output_file_name;

  vector<string> modified_lines;

  static int assembling_finished;

  static int locationCounter;
  static string pendingSectionName;
  static int pendingSectionNo;

  static int sections_Id;
  static int symbols_Id;

  string removeCommentsFromLine(string line);
  string removeBlankCharactersAtBeginningFromLine(string line);
  string removeAdditionalSpacesFromLine(string line);
  string removePunctuationSpaceFromLine(string line);

  bool checkForLabelInLine(string modified_line);
  bool checkForDirectiveInLine(string  modified_line);
  bool checkForInstructionInLine1(string modified_line);
  bool checkForInstructionInLine2(string modified_line);

  bool checkIfGlobalDirective(string line);
  bool checkIfExternDirective(string line);
  bool checkIfSectionDirective(string line);
  bool checkIfWordDirective1(string line);
  bool checkIfWordDirective2(string line);
  bool checkIfSkipDirective1(string line);
  bool checkIfSkipDirective2(string line);
  bool checkIfEndDirective(string line);

  void isThereUndefinedSymbol();
  void dodajRelokacijuZaSekciju(int offset, int symbol, int type, int tekucaSekcija, int addend);

//proveri
  int parseModifiedLines();

  void checkIfPollExist();
  //vraca adresu simbola u bazenu literala
  int symOperandForJmpAndCall(EntrySymbolTable&);
  int litOperandForJmpAndCall(string s);

  string intUHex(int broj);
  int hexUInt(string hex);
  string stringToUpper(string strToConvert);
  string simbolHexOblik(string hexVrednost);

  void skipTheLiteralPool();

  bool first_pass();
  bool second_pass();

public:
  static int lineCounter;
  map<string, EntrySectionTable> tableSections;
  map<string,EntrySymbolTable> tableSymbols;
  vector<EntryRealocationTable> relocation_table;
  map<string, list<EntryLiteralPool>> sectionToLiteralPool;


  Assembler(string inputFileName, string outputFileName);
  void ispisRelokacioneZapise();//proveri vrv necu implementirati
  int checkFileExtensions();
  void initializeAssembler();
  int makeAssemblyLines();
  int assembly();
  void generatorOutputFile();

};

#endif
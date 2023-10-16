#include <iomanip>
#include <sstream>
#include <cctype>

#include "../inc/assembler.h"
#include "../inc/hex.h"
#include "../inc/regex.h"



int Assembler::sections_Id = 0;
int Assembler::symbols_Id = 0;
int Assembler::lineCounter = 0;
string Assembler::pendingSectionName = "";
int Assembler::pendingSectionNo = -1;
int Assembler::assembling_finished = 0;
int Assembler::locationCounter = 0;
bool ok = false;


void Assembler::skipTheLiteralPool(){
  if (sectionToLiteralPool.find(pendingSectionName) == sectionToLiteralPool.end()) return;
  //proveri
  if (sectionToLiteralPool[pendingSectionName].empty()) return; // ne bi trebalo nikad da se izvrsi
  int poolSize = sectionToLiteralPool[pendingSectionName].size() * 4; // za svaki ulaz ostavljena 4 bajta
  int jumpOver = 0b00110000111100000000 << 12;
  jumpOver = jumpOver | (poolSize & 0xFFF); // PC + DISP(poolSize*4)
  tableSections[pendingSectionName].offsets.push_back(locationCounter);
  tableSections[pendingSectionName].data.push_back(jumpOver >> 24);
  tableSections[pendingSectionName].data.push_back(jumpOver >> 16);
  tableSections[pendingSectionName].data.push_back(jumpOver >> 8);
  tableSections[pendingSectionName].data.push_back(jumpOver);
  locationCounter += 4;

}

int getDiscplacement(int instr, int opaddr){
  int displ = opaddr - instr;
  displ = displ & 0x00000FFF;
  return displ;
}

int pretvoriUDecimalnuVrednost(string s){
  int literal;
  if (s.size() > 1){
    if (s[1] == 'x'){
      literal = hexadecimalToDecimal(s.substr(2));
      //proveri
      //stringstream ss;
      /*ss << s.substr(2);
      ss >> hex >> literal;*/
      //ss << hex << s;
      //ss >> literal;
      //cout << literal << endl;
      return literal;
    } 
  }
  // decimalna vrednost
  literal = stoi(s);
  //cout << literal <<endl;
  
  return literal;
}

void Assembler::checkIfPollExist(){
  // inicijalno ostavljamo mesta za preskakanje bazena literala jmp instrukcijom
  // znacu kolko treba da preskocim kad uzmem sectionToLiteralPool[section].size()?
  tableSections[pendingSectionName].size += 4;
  if (sectionToLiteralPool.find(pendingSectionName) == sectionToLiteralPool.end()){
    //cout<< "Napravljen pool za " << pendingSectionName << endl;
    tableSections[pendingSectionName].size += 4;
    list<EntryLiteralPool> list = {};
    sectionToLiteralPool.insert({pendingSectionName, list});
  } else {
    // postoji vec lista
  }
}

string removeBlankCharactersAtBeginninFromWord(string w){
  int position = w.find_first_not_of(' ');
  string newString = w;
  if(position != 0){
    newString = w.erase(0, position);
  }
  return newString;
}

void tokenize(string const &str, const char delim, vector<string> &out)
{
    stringstream ss(str);
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
    for (auto &s: out) {
      s = removeBlankCharactersAtBeginninFromWord(s);
    }
}

Assembler::Assembler(string inputFileName, string outputFileName){
  input_file_name = inputFileName;
  output_file_name = outputFileName;
}

int Assembler::checkFileExtensions(){
  if(regex_match(input_file_name, regex_input_file_extension)){
    //cout << "[ Prosao regex za ulazni fajl ]" << endl;
  }else{
    cout << "*** Ulazni fajl nema ekstenziju .s ***" << endl;
    return - 1;
  }
  if(regex_match(output_file_name, regex_ouput_file_extension)){
    //cout << "[ Prosao regex za izlazni fajl ]" << endl;    
  }else{
    cout << "*** Izlazni fajl nema ekstenziju .o ***" << endl;
    return - 1;
  }
  return 0;
}

void Assembler::initializeAssembler(){

  /*EntrySectionTable nedefinisanaSekcija;
  nedefinisanaSekcija.id_section = sections_Id++;
  nedefinisanaSekcija.section_name = "UNDEFINED";
  nedefinisanaSekcija.size = 0;
  nedefinisanaSekcija.beginnningAddress = 0;
  tableSections["UNDEFINED"] = nedefinisanaSekcija;

  EntrySymbolTable nedefinisanSimbol;
  nedefinisanSimbol.id_symbol = symbols_Id++;
  nedefinisanSimbol.is_defined = true;
  nedefinisanSimbol.is_extern = false;
  nedefinisanSimbol.is_local = true;
  nedefinisanSimbol.name = "UNDEFINED";
  nedefinisanSimbol.section = "UNDEFINED";
  nedefinisanSimbol.value = 0;
  nedefinisanSimbol.size = 0;
  nedefinisanSimbol.isSection = true;
  tableSymbols["UNDEFINED"] = nedefinisanSimbol;*/
  
}

int Assembler::makeAssemblyLines(){
  ifstream inputFile;
  inputFile.open(input_file_name);
  if (inputFile.is_open()){
    string currentLine;
    while (getline(inputFile, currentLine)){
      lineCounter++;
      currentLine = removeBlankCharactersAtBeginningFromLine(currentLine);
      currentLine = removeCommentsFromLine(currentLine);
      currentLine = removeAdditionalSpacesFromLine(currentLine);
      currentLine = removePunctuationSpaceFromLine(currentLine);
      if (!regex_match(currentLine, regex_empty_line)){
        int pos = currentLine.find_last_not_of(" ");
        currentLine = currentLine.substr(0, pos+1);
        modified_lines.push_back(currentLine);
      }
    }
    inputFile.close();
    return 0;
  } else {
    cout << "Greska pri otvaranju ulaznog fajla" << endl;
    return -1;
  }
}


string Assembler::removeBlankCharactersAtBeginningFromLine(string line){
  int position = line.find_first_not_of(' ');
  string newString = line;
  if(position != 0){
    newString = line.erase(0, position);
  }
  return newString;
}

string Assembler::removeAdditionalSpacesFromLine(string line){
  string newLine = regex_replace(line, regex_additionalSpace, " ");
  return newLine;
}

string Assembler::removePunctuationSpaceFromLine(string line){
  string newLine = regex_replace(line, regex_punctuationSpace, ", ");
  return newLine;
}

string Assembler::removeCommentsFromLine(string line){
  int position = line.find_first_of('#');
  string newString = "";
  if(position != 0){
    newString = line.substr(0, position-1);
  }
  return newString;
}

void Assembler::generatorOutputFile(){
  ofstream text_output_file(output_file_name);
  int addr = 0;
  int pool = 0;
  for (map <string, EntrySectionTable>::iterator it = tableSections.begin(); it != tableSections.end(); it++){
    addr = 0;
    pool = 0;
    uint8_t word[4];
    if (!sectionToLiteralPool[it->first].empty()) pool = sectionToLiteralPool[it->first].front().address;
    text_output_file << "\nsection " << it->second.id_section << ":" << "\n";
    for (uint8_t byte : tableSections[it->first].data){
      word[3-addr%4] = byte;
      if (addr % 4 == 0) text_output_file << addr << ": ";
      if (addr % 4 == 3 ) text_output_file << wordToHex(word) << "\n";
      addr++;
      // ako ima bazen literala i ako smo stigli do njega(kraj sekcije)
      if (addr == pool) {
        for (auto it2 = sectionToLiteralPool[it->first].begin(); it2 != sectionToLiteralPool[it->first].end(); it2++){
          text_output_file << addr << ": " << translateToHexLittleEndian((unsigned int)(it2->value)) << "\n";
          addr += 4;
        }
      }
    }
  }
  text_output_file << "\nsection_table:\n";
  int cnt = 0;
  for(auto it = tableSections.begin(); it != tableSections.end(); it++){
    text_output_file << cnt++ << ": " << it->first << " " << it->second.size << "\n"; 
  }
  text_output_file << "\nsymbol_table:\n";
  for(auto it = tableSymbols.begin(); it != tableSymbols.end(); it++){
    string type = "";
    if (it->second.is_extern) type = "extern";
    else if (it->second.is_local) type = "local";
    else if (it->second.is_defined && !it->second.is_local) type = "global";
    text_output_file << tableSections[it->second.section].id_section << " " << type << " " << it->first << " " << it->second.value << " " << it->second.is_defined << "\n";
  }
  text_output_file << "\nrelocation_table:\n";
  for (auto it = relocation_table.begin(); it != relocation_table.end(); it++){
    text_output_file << tableSections[it->section_name].id_section << ":" << it->offset << ":" << it->symbol_name << "\n";
  }
  text_output_file.close();

}

int Assembler::assembly(){
  int ret = makeAssemblyLines();
  if(ret < 0){
    return -1;
  }
  if (!first_pass()){
    return -1;
  }
  if (!second_pass()){
    return -1;
  }

  generatorOutputFile();

  return 0;
}

bool Assembler::checkForLabelInLine(string line) {
  smatch sm;
  if (regex_search(line, sm, regex_for_label)){
    ok = true;
    string label = sm.str(1);

    if (pendingSectionName == ""){
      return false;
    }
    map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(label);
    if (sym != tableSymbols.end()){
      if (sym->second.is_defined == true) return false;
      if (sym->second.is_extern == true) return false;
      sym->second.is_defined = true;
      sym->second.value = locationCounter;
      sym->second.section = pendingSectionName;
    } else {
      EntrySymbolTable sym;
      sym.id_symbol = symbols_Id++;
      sym.is_defined = true;
      sym.is_extern = false;
      sym.is_local = true;
      sym.name = label;
      sym.section = pendingSectionName;
      sym.value = locationCounter;
      tableSymbols[label] = sym;
    }
  }
  return true;
}

bool Assembler::checkIfGlobalDirective(string line){
  smatch sm;
  if (regex_search(line, sm, regex_for_global)){
    ok = true;
    //proveri
    string simboli = sm.str(1);
    const char delimiter = ',';
    vector<string> simbol;
    tokenize(simboli, delimiter, simbol);
    //proveri
    for (auto & s : simbol){
      map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(s);
      if (sym != tableSymbols.end()){
        sym->second.is_local = false;
        // it can be global and also both defined (is_defined = true) or nondefined (is_extern = true)
      }
      else {
        EntrySymbolTable sym;
        sym.id_symbol = symbols_Id++;
        sym.is_defined = false;
        sym.is_extern = false;
        sym.is_local = false;
        sym.name = s;
        sym.section = "UNDEFINED";
        sym.value = 0;
        tableSymbols[s] = sym;
      }
    }
    return true;
  }
  return true;
}

bool Assembler::checkIfExternDirective(string line){
  smatch sm;
  if (regex_search(line, sm, regex_for_extern)){
    ok = true;
    //proveri, mozda treba line.substr(8, -1)
    string simboli = line.substr(8);
    const char delimiter = ',';
    vector<string> simbol;
    tokenize(simboli, delimiter, simbol);
    for (auto & s: simbol){
      map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(s);
      if (sym != tableSymbols.end()){
        if (sym->second.is_defined == true){
          return false;
        }
      } else {
        EntrySymbolTable sym;
        sym.id_symbol = symbols_Id++;
        sym.is_defined = false;
        sym.is_local = false;
        sym.is_extern = true;
        sym.name = s;
        sym.section = "UNDEFINED";
        sym.value = 0;
        tableSymbols[s] = sym;
      }
    }
    return true;
  }
  return true;
}

bool Assembler::checkIfSectionDirective(string line){
  smatch sm;
  if (regex_search(line, sm, regex_for_section)){
    ok = true;
    //proveri da l je moglo sm.str(1) a da u regexu deo nakon .section stavim u ()
    string secName = sm.str(1);

    locationCounter = 0;
    pendingSectionName = secName;

    /*EntrySymbolTable secSym;
    secSym.id_symbol = symbols_Id++;
    secSym.is_defined = true;
    secSym.is_extern = false;
    secSym.is_local = true;
    secSym.name = secName;
    secSym.section = secName;
    secSym.value = locationCounter;
    tableSymbols[secName] = secSym;*/

    EntrySectionTable sec;
    sec.id_section = sections_Id++;
    sec.section_name = secName;
    sec.size = 0;
    tableSections[secName] = sec;
    return true;
  }
  return true;
}

bool Assembler::checkIfSkipDirective1(string line){
  smatch sm;
  int literal;
  bool sk = false;
  if (regex_search(line, sm, regex_for_skip_dec_literal)){
    ok = true;
    if (pendingSectionName == "") return false;
    string bytesForSkipString = sm.str(1);
    literal = stoi(bytesForSkipString);
    sk = true;
  } //proveri trebalo bi da ne mora else i da je sve ok
  else if (regex_search(line, sm, regex_for_skip_hex_literal)){
    ok = true;
    if (pendingSectionName == "") return false;
    //string bytesForSkipString = line.substr(8);
    stringstream ss;
    ss << sm.str(1);
    ss >> hex >> literal;
    sk = true;
  }

  if (sk){
    if (literal < 0) return false;
    int more = literal % 4;
    if (more == 0) more = 4;
    locationCounter += (literal + 4 - more);
    tableSections[pendingSectionName].size += literal + 4 - more;
  }
  return true;
}

bool Assembler::checkIfEndDirective(string line){
  smatch sm;
  if (regex_search(line, sm, regex_for_end)){
    ok = true;
    Assembler::assembling_finished = 1;
    return true;
  }
  return false;
}

bool Assembler::checkIfWordDirective1(string line){
  smatch sm;
  if (regex_search(line, sm, regex_for_word)) {
    ok = true;
    if (pendingSectionName == "") return false;
    //proveri
    string simboli = line.substr(6);
    const char delimiter = ',';
    vector<string> simbol;
    tokenize(simboli, delimiter, simbol);
    for (auto & s: simbol){
      locationCounter += 4;
      tableSections[pendingSectionName].size += 4;
    }
  }
  return true;
}

bool Assembler::checkForInstructionInLine1(string line){
  smatch sm;
  if (regex_search(line, sm , regex_for_instruction_with_no_operands)){
    //if (pendingSectionName == "") return false;
    if (sm.str(1) == "iret"){
      locationCounter += 4;
      tableSections[pendingSectionName].size += 4;
    }
  }
  if (regex_search(line, sm, regex_for_ld_instruction)){
    string op = sm.str(1);
    if (isalpha(op[0]) != 0) {
      locationCounter += 4;
      tableSections[pendingSectionName].size += 4;
    } else if (isdigit(op[0]) != 0) {
      locationCounter += 4;
      tableSections[pendingSectionName].size += 4;
    }
  }
  if (regex_search(line, sm, regex_for_instruction_with_no_operands)
      || regex_search(line, sm, regex_for_instruction_with_one_operand)
      || regex_search(line, sm, regex_for_one_operand_jump_instruction)
      || regex_search(line, sm, regex_for_branch_instruction)
      || regex_search(line, sm, regex_for_ld_instruction)
      || regex_search(line, sm, regex_for_st_instruction)
      || regex_search(line, sm, regex_for_csrrd_instruction)
      || regex_search(line, sm, regex_for_csrwr_instruction)
      || regex_search(line, sm, regex_for_other_instructions_with_two_operands)){
        ok = true;
        if (pendingSectionName == "") return false;
        locationCounter += 4;
        tableSections[pendingSectionName].size += 4;
  }
  return true;

  //ako nije nijedna od ovih

}

bool Assembler::first_pass(){
  //cout<<"Usao sam u prvi prolaz"<<endl;
  for (size_t i = 0 ; i < modified_lines.size(); i++){
    ok = false;
    lineCounter++;
    if (!checkForLabelInLine(modified_lines[i])){
      return false;
    }
    if (!checkIfGlobalDirective(modified_lines[i])){
      return false;
    }
    if (!checkIfExternDirective(modified_lines[i])){
      return false;
    }
    if (!checkIfSectionDirective(modified_lines[i])){
      return false;
    }
    if (!checkIfSkipDirective1(modified_lines[i])){
      return false;
    }
    if (checkIfEndDirective(modified_lines[i])){
      break;
    }
    if (!checkIfWordDirective1(modified_lines[i])){
      return false;
    }
    //Instrukcija je u pitanju
    if (!checkForInstructionInLine1(modified_lines[i])){
      return false;
    }
    //cout << i+1 << "/" << modified_lines.size() <<endl;
    if (!ok) exit(-1); // neispravan zapis u izvornom fajlu
  }
  return true;
}


bool Assembler::checkIfSkipDirective2(string line){
  smatch sm;
  int literal;
  bool sk = false;
  if (regex_search(line, sm, regex_for_skip_dec_literal)){
    string bytesForSkipString = line.substr(6);
    literal = stoi(bytesForSkipString);
    sk = true;
  }
  else if (regex_search(line, sm, regex_for_skip_hex_literal)){
    stringstream ss;
    ss << line.substr(8);
    ss >> hex >> literal;
    sk = true;
  }
  if (sk){
    
    int more = literal % 4;
    if (more == 0) more = 4;
    for (size_t i = 0; i < literal + 4 - more; i++){
      if ((i % 4) == 0) tableSections[pendingSectionName].offsets.push_back(locationCounter);
      tableSections[pendingSectionName].data.push_back(0);
      locationCounter += 1;
    }
    return true;
  }
  return false;
}

bool Assembler::checkIfWordDirective2(string line){
  smatch sm;
  if (regex_search(line, sm, regex_for_word)) {
    string simboli = sm.str(1);
    const char delimiter = ',';
    vector<string> simbol;
    tokenize(simboli, delimiter, simbol);
    for (auto &s: simbol){
      // u pitanju je simbol jer pocinje slovom
      if (isalpha(s[0]) != 0){
        map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(s);
        if (sym != tableSymbols.end()){
          //EntrySymbolTable est = sym->second;
          if (sym->second.is_defined){
            if (sym->second.is_local){
              tableSections[pendingSectionName].offsets.push_back(locationCounter);
              tableSections[pendingSectionName].data.push_back((sym->second.value >> 24) & 0xff);
              tableSections[pendingSectionName].data.push_back((sym->second.value >> 16) & 0xff);
              tableSections[pendingSectionName].data.push_back((sym->second.value >> 8) & 0xff);
              tableSections[pendingSectionName].data.push_back(sym->second.value & 0xff);

              EntryRealocationTable rel_data;
              rel_data.is_data = true;
              rel_data.type = "R_HYP_32";
              rel_data.section_name = pendingSectionName;
              rel_data.offset = locationCounter;
              rel_data.symbol_name = sym->second.section; // this is local symbol, so only section is necessary !
              rel_data.addend = 0;
              relocation_table.push_back(rel_data);
            }
            else {
              // global symbol, add zeros to data
              tableSections[pendingSectionName].offsets.push_back(locationCounter);
              tableSections[pendingSectionName].data.push_back(0 & 0xff);
              tableSections[pendingSectionName].data.push_back((0 >> 8) & 0xff);
              tableSections[pendingSectionName].data.push_back((0 >> 16) & 0xff);
              tableSections[pendingSectionName].data.push_back((0 >> 24) & 0xff);

              EntryRealocationTable rel_data;
              rel_data.is_data = true;
              rel_data.type = "R_HYP_32";
              rel_data.section_name = pendingSectionName;
              rel_data.offset = locationCounter;
              rel_data.symbol_name = sym->second.name;
              rel_data.addend = 0;
              relocation_table.push_back(rel_data);
            }
          } else {
            //extern symbol
            tableSections[pendingSectionName].offsets.push_back(locationCounter);
            tableSections[pendingSectionName].data.push_back(0 & 0xff);
            tableSections[pendingSectionName].data.push_back((0 >> 8) & 0xff);
            tableSections[pendingSectionName].data.push_back((0 >> 16) & 0xff);
            tableSections[pendingSectionName].data.push_back((0 >> 24) & 0xff);

            EntryRealocationTable rel_data;
            rel_data.is_data = true;
            rel_data.type = "R_HYP_32";
            rel_data.section_name = pendingSectionName;
            rel_data.offset = locationCounter;
            rel_data.symbol_name = sym->second.name;
            rel_data.addend = 0;
            relocation_table.push_back(rel_data);

          }
        } else {
          // exit (-1);
          return false;
        }
      } else {
        // u pitanju literal a ne simbol
        int literal;
        //proveri da li je u pitanju hex
        
        literal = pretvoriUDecimalnuVrednost(s);
        
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back((literal >> 24) & 0xff);
        tableSections[pendingSectionName].data.push_back((literal >> 16) & 0xff);
        tableSections[pendingSectionName].data.push_back((literal >> 8) & 0xff);
        tableSections[pendingSectionName].data.push_back(literal & 0xff);
        
      }
      locationCounter += 4;
    }
    return true;
  }
  return false;
}

int Assembler::symOperandForJmpAndCall(EntrySymbolTable& second){
  checkIfPollExist();
  EntryLiteralPool ent;
  
  if (sectionToLiteralPool[pendingSectionName].empty()){
    ent.address = tableSections[pendingSectionName].size - 4;
  } else {
    ent.address = sectionToLiteralPool[pendingSectionName].back().address + 4;
  }
  // ako je extern simbol u bazen ide 0
  if (second.is_extern) ent.value = 0;
  if (second.section == pendingSectionName) ent.value = second.value;
  else ent.value = 0;
  sectionToLiteralPool[pendingSectionName].push_back(ent);
  EntryRealocationTable rel_data;
  rel_data.is_data = false;
  rel_data.addend = 0;
  rel_data.type = "R_HYP_32";
  rel_data.offset = ent.address; // prvi bajt simbola u bazenu
  rel_data.section_name = pendingSectionName;
  rel_data.symbol_name = second.name;
  relocation_table.push_back(rel_data);

  return ent.address;
}

int Assembler::litOperandForJmpAndCall(string operand){
  //cout << "Operand :" << operand << endl;
  int literal = pretvoriUDecimalnuVrednost(operand);
  checkIfPollExist();
  EntryLiteralPool ent;
  //proveri
  if (sectionToLiteralPool[pendingSectionName].empty()){
    ent.address = tableSections[pendingSectionName].size - 4;
  } else {
    ent.address = sectionToLiteralPool[pendingSectionName].back().address + 4;
  }
  //cout << "Literali :" << translateToHexLittleEndian(literal) << endl;
  ent.value = literal;
  sectionToLiteralPool[pendingSectionName].push_back(ent);

  return ent.address;
}

bool Assembler::checkForInstructionInLine2(string line){
  smatch sm;
  if (regex_search(line, sm, regex_for_instruction_with_no_operands)){
    string instructionName = sm.str(1);
    if (!instructionName.compare("halt")){
      
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x00);
      locationCounter += 4;
    } else if (!instructionName.compare("iret")){
      // moracemo preko 2 instrukcije da izvrsimo ovo
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      // prvo cu da skinem status i ako ne bi trebalo tako, al zaemnicu im redosled
      // pushovanja na stek pri ulasku u prekidnu rutinu
      // instr ucitavanja podatka (9) i modif je 7 -> 0x97
      tableSections[pendingSectionName].data.push_back(0x97);
      // status reg je 0, sp je r14 -> 0x0E
      tableSections[pendingSectionName].data.push_back(0x0E);
      tableSections[pendingSectionName].data.push_back(0x00);
      // ovo 0x04 je nizih 8b displacmenta, a on je ovde pomeraj za sp
      tableSections[pendingSectionName].data.push_back(0x04);
      locationCounter += 4;
      
      //proveri
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      // hocu da skinem i pc sa steka
      // opet instr ucitavanja podatka (9) ali sad modif 3 -> 0x93
      tableSections[pendingSectionName].data.push_back(0x93);
      // pc reg je r15, sp je r14 -> 0xFE
      tableSections[pendingSectionName].data.push_back(0xFE);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x04);
      locationCounter += 4;

    } else if (!instructionName.compare("ret")){
      // mislim da je ista kao pop pc iz iret dela
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      // hocu da skinem pc sa steka
      // opet instr ucitavanja podatka (9) ali sad modif 3 -> 0x93
      tableSections[pendingSectionName].data.push_back(0x93);
      // pc reg je r15, sp je r14 -> 0xFE
      tableSections[pendingSectionName].data.push_back(0xFE);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x04);
      locationCounter += 4;
    } else if (!instructionName.compare("int")){
      // valjda cu ovde samo da stavim kod instrukcije, a u emulatoru da izvrsim sve ono sto treba
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      tableSections[pendingSectionName].data.push_back(0x10);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x00);
      locationCounter += 4;
    }
  } else if (regex_search(line, sm, regex_for_instruction_with_one_operand)){
    // ovo su push pop i not instr
    string instructionName = sm.str(1);
    int regNum;
    if (sm.str(2) == "pc"){
      regNum = 15;
    } else if (sm.str(2) == "sp"){
      regNum = 14;
    } else {
      regNum = stoi(sm.str(2).substr(1));
    }

    if (!instructionName.compare("push")){
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      //instrukcija smestanja podataka (8) i modif 1 -> 0x81
      tableSections[pendingSectionName].data.push_back(0x81);
      // u A reg smestimo sp (14) u B 0 -> 0xE0
      tableSections[pendingSectionName].data.push_back(0xE0);
      //preoveri je l dobro stavljen registar koji je naveden u inst
      tableSections[pendingSectionName].data.push_back((regNum << 4) + 15);
      tableSections[pendingSectionName].data.push_back(0xFC);
      locationCounter += 4;
    } else if (!instructionName.compare("pop")){
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      tableSections[pendingSectionName].data.push_back(0x93);
      tableSections[pendingSectionName].data.push_back((regNum << 4) + 14);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x04);
      locationCounter += 4;
    } else if (!instructionName.compare("not")) {
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      tableSections[pendingSectionName].data.push_back(0x60);
      tableSections[pendingSectionName].data.push_back((regNum << 4) + regNum);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x00);
      locationCounter += 4;
    }
  } else if (regex_search(line, sm, regex_for_one_operand_jump_instruction)){
    string instructionName = sm.str(1);
    string operand = sm.str(2);
    
    if (!instructionName.compare("call")){
      // CallSymbol
      if (isalpha(operand[0]) != 0){
        map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(operand);
        if (sym != tableSymbols.end()){
          int entAdr = symOperandForJmpAndCall(sym->second);

          tableSections[pendingSectionName].offsets.push_back(locationCounter);
          // poziv potprograma memorijskim adresiranjem (pravim PC rel prakticno)
          tableSections[pendingSectionName].data.push_back(0x21);
          tableSections[pendingSectionName].data.push_back(0xF0);
          // oduzima se vrednost pocetka kraja instrukcije i mesta simbola u bazenu
          char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
          tableSections[pendingSectionName].data.push_back(d1);
          d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
          tableSections[pendingSectionName].data.push_back(d1);

        }
        else {
          // simbol mora biti u tabeli sim
          exit(-1);
        }
      } else {
        // CallLiteral
        int entAdr = litOperandForJmpAndCall(operand);
        
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x21);
        tableSections[pendingSectionName].data.push_back(0xF0);
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back(d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);
      }
    } else if (!instructionName.compare("jmp")){
      //JmpSymbol
      if (isalpha(operand[0]) != 0){
        map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(operand);
        if (sym != tableSymbols.end()){
          int entAdr = symOperandForJmpAndCall(sym->second);
          tableSections[pendingSectionName].offsets.push_back(locationCounter);
          tableSections[pendingSectionName].data.push_back(0x38);
          tableSections[pendingSectionName].data.push_back(0xF0);
          char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
          tableSections[pendingSectionName].data.push_back(d1);
          d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
          tableSections[pendingSectionName].data.push_back(d1);
        } else {
          // simbol mora biti u tabeli sim
          exit(-1);
        }
      } else {
        //JmpLiteral
        int entAdr = litOperandForJmpAndCall(operand);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x38);
        tableSections[pendingSectionName].data.push_back(0xF0);
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back(d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);
      }
    }
    locationCounter += 4;

  } else if (regex_search(line, sm, regex_for_other_instructions_with_two_operands)){
    // ARITMETICKE I LOGICKE OPERACIJE
    string instructionName = sm.str(1);
    int regDNum;
    int regSNum;
    if (sm.str(2) == "pc"){
      regSNum = 15;
    } else if (sm.str(2) == "sp"){
      regSNum = 14;
    } else {
      regSNum = stoi(sm.str(2).substr(1));
    }

    if (sm.str(3) == "pc"){
      regDNum = 15;
    } else if (sm.str(3) == "sp"){
      regDNum = 14;
    } else {
      regDNum = stoi(sm.str(3).substr(1));
    }

    tableSections[pendingSectionName].offsets.push_back(locationCounter);

    if (!instructionName.compare("xchg")){
      // OC je 0x40
      tableSections[pendingSectionName].data.push_back(0x40);
      // za A je 0, a za B ide regDestination -> (0 << 4) + regDNum
      tableSections[pendingSectionName].data.push_back(regDNum);
      // za C je regSource a D je 0
      tableSections[pendingSectionName].data.push_back((regSNum << 4) + 0);
      tableSections[pendingSectionName].data.push_back(0x00);

    } else if (!instructionName.compare("add")){
      // OC je 0x50
      tableSections[pendingSectionName].data.push_back(0x50);
      tableSections[pendingSectionName].data.push_back((regDNum << 4) + regDNum);
      tableSections[pendingSectionName].data.push_back((regSNum << 4) + 0);
      tableSections[pendingSectionName].data.push_back(0x00);

    } else if (!instructionName.compare("sub")){
      tableSections[pendingSectionName].data.push_back(0x51);
      tableSections[pendingSectionName].data.push_back((regDNum << 4) + regDNum);
      tableSections[pendingSectionName].data.push_back((regSNum << 4) + 0);
      tableSections[pendingSectionName].data.push_back(0x00);

    } else if (!instructionName.compare("mul")){
      tableSections[pendingSectionName].data.push_back(0x52);
      tableSections[pendingSectionName].data.push_back((regDNum << 4) + regDNum);
      tableSections[pendingSectionName].data.push_back((regSNum << 4) + 0);
      tableSections[pendingSectionName].data.push_back(0x00);

    } else if (!instructionName.compare("div")){
      tableSections[pendingSectionName].data.push_back(0x53);
      tableSections[pendingSectionName].data.push_back((regDNum << 4) + regDNum);
      tableSections[pendingSectionName].data.push_back((regSNum << 4) + 0);
      tableSections[pendingSectionName].data.push_back(0x00);

    } else if (!instructionName.compare("and")){
      tableSections[pendingSectionName].data.push_back(0x61);
      tableSections[pendingSectionName].data.push_back((regDNum << 4) + regDNum);
      tableSections[pendingSectionName].data.push_back((regSNum << 4) + 0);
      tableSections[pendingSectionName].data.push_back(0x00);

    } else if (!instructionName.compare("or")){
      tableSections[pendingSectionName].data.push_back(0x62);
      tableSections[pendingSectionName].data.push_back((regDNum << 4) + regDNum);
      tableSections[pendingSectionName].data.push_back((regSNum << 4) + 0);
      tableSections[pendingSectionName].data.push_back(0x00);

    } else if (!instructionName.compare("xor")){
      tableSections[pendingSectionName].data.push_back(0x63);
      tableSections[pendingSectionName].data.push_back((regDNum << 4) + regDNum);
      tableSections[pendingSectionName].data.push_back((regSNum << 4) + 0);
      tableSections[pendingSectionName].data.push_back(0x00);

    }
    locationCounter += 4;
  } else if (regex_search(line, sm, regex_for_csrrd_instruction)){
    int csr;
    int reg;
    if (sm.str(1) == "status"){
      csr = 0;
    } else if (sm.str(1) == "handler"){
      csr = 1;
    } else if (sm.str(1) == "cause"){
      csr = 2;
    }
    if (sm.str(2) == "sp"){
      reg = 14;
    } else if (sm.str(2) == "pc"){
      reg = 15;
    } else {
      reg = stoi(sm.str(2).substr(1));
    }
    tableSections[pendingSectionName].offsets.push_back(locationCounter);
    tableSections[pendingSectionName].data.push_back(0x90);
    tableSections[pendingSectionName].data.push_back((reg << 4) + csr);
    tableSections[pendingSectionName].data.push_back(0x00);
    tableSections[pendingSectionName].data.push_back(0x00);
    locationCounter += 4;
  } else if (regex_search(line, sm, regex_for_csrwr_instruction)){
    int csr;
    int reg;
    if (sm.str(2) == "status"){
      csr = 0;
    } else if (sm.str(2) == "handler"){
      csr = 1;
    } else if (sm.str(2) == "cause"){
      csr = 2;
    }
    if (sm.str(1) == "sp"){
      reg = 14;
    } else if (sm.str(1) == "pc"){
      reg = 15;
    } else {
      reg = stoi(sm.str(1).substr(1));
    }
    tableSections[pendingSectionName].offsets.push_back(locationCounter);
    tableSections[pendingSectionName].data.push_back(0x94);
    tableSections[pendingSectionName].data.push_back((csr << 4) + reg);
    tableSections[pendingSectionName].data.push_back(0x00);
    tableSections[pendingSectionName].data.push_back(0x00);
    locationCounter += 4;
  } else if (regex_search(line, sm, regex_for_branch_instruction)){
    string instructionName = sm.str(1);
    int reg1;
    int reg2;
    string operand = sm.str(4);
    if (sm.str(2) == "sp"){
      reg1 = 14;
    } else if (sm.str(2) == "pc"){
      reg1 = 15;
    } else {
      reg1 = stoi(sm.str(2).substr(1));
    }
    if (sm.str(3) == "sp"){
      reg2 = 14;
    } else if (sm.str(3) == "pc"){
      reg2 = 15;
    } else {
      reg2 = stoi(sm.str(3).substr(1));
    }
    if (!instructionName.compare("beq")){
      //Beq symbol
      if (isalpha(operand[0]) != 0){
        map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(operand);
        if (sym != tableSymbols.end()){
          int entAdr = symOperandForJmpAndCall(sym->second);
          tableSections[pendingSectionName].offsets.push_back(locationCounter);
          tableSections[pendingSectionName].data.push_back(0x39);
          tableSections[pendingSectionName].data.push_back((15 << 4) + reg1);
          //proveri
          char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
          tableSections[pendingSectionName].data.push_back((reg2 << 4) + d1);
          d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
          tableSections[pendingSectionName].data.push_back(d1);
        } else {
          // simbol mora biti u tabeli sim
          exit(-1);
        }
      } else {
        //Beq literal
        int entAdr = litOperandForJmpAndCall(operand);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x39);
        tableSections[pendingSectionName].data.push_back((15 << 4) + reg1);
        //proveri
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back((reg2 << 4) + d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);
      }
    } else if (!instructionName.compare("bne")){
      if (isalpha(operand[0]) != 0){
        //Bne symbol
        map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(operand);
        if (sym != tableSymbols.end()){
          int entAdr = symOperandForJmpAndCall(sym->second);
          tableSections[pendingSectionName].offsets.push_back(locationCounter);
          tableSections[pendingSectionName].data.push_back(0x3A);
          tableSections[pendingSectionName].data.push_back((15 << 4) + reg1);
          //proveri
          char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
          tableSections[pendingSectionName].data.push_back((reg2 << 4) + d1);
          d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
          tableSections[pendingSectionName].data.push_back(d1);
        } else {
          //Bne literal
          int entAdr = litOperandForJmpAndCall(operand);
          tableSections[pendingSectionName].offsets.push_back(locationCounter);
          tableSections[pendingSectionName].data.push_back(0x3A);
          tableSections[pendingSectionName].data.push_back((15 << 4) + reg1);
          //proveri
          char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
          tableSections[pendingSectionName].data.push_back((reg2 << 4) + d1);
          d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
          tableSections[pendingSectionName].data.push_back(d1);
        }
      }
    } else if (!instructionName.compare("bgt")){
      if (isalpha(operand[0]) != 0){
        //Bgt symbol
        map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(operand);
        if (sym != tableSymbols.end()){
          int entAdr = symOperandForJmpAndCall(sym->second);
          tableSections[pendingSectionName].offsets.push_back(locationCounter);
          tableSections[pendingSectionName].data.push_back(0x3B);
          tableSections[pendingSectionName].data.push_back((15 << 4) + reg1);
          //proveri
          char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
          tableSections[pendingSectionName].data.push_back((reg2 << 4) + d1);
          d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
          tableSections[pendingSectionName].data.push_back(d1);
        } else {
          //Bgt literal
          int entAdr = litOperandForJmpAndCall(operand);
          tableSections[pendingSectionName].offsets.push_back(locationCounter);
          tableSections[pendingSectionName].data.push_back(0x3B);
          tableSections[pendingSectionName].data.push_back((15 << 4) + reg1);
          //proveri
          char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
          tableSections[pendingSectionName].data.push_back((reg2 << 4) + d1);
          d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
          tableSections[pendingSectionName].data.push_back(d1);
        }
      }
    }
    locationCounter += 4;
  } else if (regex_search(line, sm, regex_for_st_instruction)){
    string operand = sm.str(2);
    string re = sm.str(1);
    int reg;
    if (re == "sp"){
      reg = 14;
    } else if (re == "pc"){ 
      reg = 15;
    } else{
      reg = stoi(re.substr(1));
    }
    if (operand[0] == '$' || operand[0] == '%'){
      // nema smisla da nesto smestamo u vrednost simbola, literala ili registra
      exit(-1);
    }
    if (isalpha(operand[0]) != 0){
      // u pitanju je symbolMem
      map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(operand);
      if (sym != tableSymbols.end()){
        int entAdr = symOperandForJmpAndCall(sym->second);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x82);
        tableSections[pendingSectionName].data.push_back(0xF0);
        // proveri OBAVEZNOO
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back((reg << 4) + d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);
      }

    } else if (isdigit(operand[0]) != 0){
      // u pitanju se literalMem
      int literal = pretvoriUDecimalnuVrednost(operand);
      if (literal >= 0 && literal <= 4095) {
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x80);
        tableSections[pendingSectionName].data.push_back(0x00);
        tableSections[pendingSectionName].data.push_back((reg << 4) + (literal>>8));
        tableSections[pendingSectionName].data.push_back(literal);
      } else {
        int entAdr = litOperandForJmpAndCall(operand);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x82);
        tableSections[pendingSectionName].data.push_back(0xF0);
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back((reg << 4) + d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);
        }
    } else {
      /*
      moguce opcije:
        operand = [%r1]
        operand = [%r1 + 0x08]
        operand = [%r1 + lit]
      */
      const char delim = '+';
      vector<string> ops;
      tokenize(operand, delim, ops);
      int regNum;
      // ops[0] = [%r1], ops[0].substr(2, 5(ili vise) - 3) = r1 (r12, ako je size() 6 a ne 5)
      string r = ops[0].substr(2,ops[0].size() - 3);
      if (r == "sp") {
        regNum = 14;
      } else if (r == "pc") {
        regNum = 15;
      } else regNum = stoi(r.substr(1));
      if (ops.size() == 1){
        // u pitanju je reg mem tj operand = [%r1]
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x80);
        tableSections[pendingSectionName].data.push_back((regNum << 4) + 0);
        tableSections[pendingSectionName].data.push_back((reg << 4) + 0);
        tableSections[pendingSectionName].data.push_back(0x00);
      } else {
        // u pitanju je 
        //operand = [%r1 + 0x08] ili
        //operand = [%r1 + lit]
        // ops[0]=[%r1 , ops[1]=0x08] - ovde je tokenize skloni blanko sa pocetka reci
        string drugiOps = ops[1];
        if (isalpha(drugiOps[0]) != 0){
          // operand = [%r1 + lit], a to nije moguce jer vrednost lit znamo tek nakon linkovanja
          exit(-1);
        } else {
          //operand = [%r1 + 0x08]
          //validno samo ako je literal manji od 12b jer inace 
          //koristimo bazen literala i opet ne znamo vrednost do linkovanja
          drugiOps = drugiOps.substr(1); // da sklonim jedan blanko karakter
          int lit = pretvoriUDecimalnuVrednost(drugiOps);
          
          if(lit < 0 || lit > 4095) exit(-1); // ni ovo nije validno jer ne moze da se smesti u instrukciju
          tableSections[pendingSectionName].offsets.push_back(locationCounter);
          tableSections[pendingSectionName].data.push_back(0x80);
          tableSections[pendingSectionName].data.push_back((regNum << 4) + 0);
          //proveri
          tableSections[pendingSectionName].data.push_back((reg << 4) + lit >> 8);
          tableSections[pendingSectionName].data.push_back(lit);
        }
      }
      }
    locationCounter += 4;
  } else if (regex_search(line, sm, regex_for_ld_instruction)){
    string operand = sm.str(1);
    string re = sm.str(2);
    int reg;
    if (re == "sp") {
      reg = 14;
    } else if (re == "pc") {
      reg = 15;
    } else reg = stoi(re.substr(1));

    if (operand[0] == '$' && (isdigit(operand[1]) != 0)){
      // operand je tipa $literal ($123)
      // proveri ali trebalo bi da samo literal ciju vrednost treba smestiti u reg stavi u bazen literala
      operand = operand.substr(1); // izbacujem $
      int literal = pretvoriUDecimalnuVrednost(operand);
      if (literal >= 0 && literal <= 4095) {
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x91);
        tableSections[pendingSectionName].data.push_back((reg << 4) + 0);
        tableSections[pendingSectionName].data.push_back(literal >> 8);
        tableSections[pendingSectionName].data.push_back(literal);
      }else  {
        int entAdr = litOperandForJmpAndCall(operand);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x92);
        tableSections[pendingSectionName].data.push_back((reg << 4) + 15);
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back(d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);
      }
    } else if (operand[0] == '$'){
      //operand je tipa $symbol ($abc)
      operand = operand.substr(1); // izbacujem $
      map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(operand);
      if (sym != tableSymbols.end()){
        int entAdr = symOperandForJmpAndCall(sym->second);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x92);
        tableSections[pendingSectionName].data.push_back((reg << 4) + 15);
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back(d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);
      }
    } else if (isdigit(operand[0]) != 0) {
      // operand je tipa literal (123 ili 0x123)
      int literal = pretvoriUDecimalnuVrednost(operand);
      if (literal >= 0 && literal <= 4095) {
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x92);
        tableSections[pendingSectionName].data.push_back((reg << 4) + 0);
        tableSections[pendingSectionName].data.push_back(literal >> 8);
        tableSections[pendingSectionName].data.push_back(literal);
      } else {
        int entAdr = litOperandForJmpAndCall(operand);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x92);
        tableSections[pendingSectionName].data.push_back((reg << 4) + 15);
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back(d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);

        locationCounter += 4;

        // sad u registru imamo vrednost literala, 
        // pa treba da uradimo jos jednu inst koja ce dohvatit vrednost iz memorije sa ove adrese u reg
        // proveri
        int code2 = (0b10010010 << 24);
        code2 = code2 | (reg << 20);
        code2 = code2 | (reg << 16);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(code2 >> 24);
        tableSections[pendingSectionName].data.push_back(code2 >> 16);
        tableSections[pendingSectionName].data.push_back(code2 >> 8);
        tableSections[pendingSectionName].data.push_back(code2);
    }
    } else if (isalpha(operand[0]) != 0){
      //operand je tipa symbol (abc)
      map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(operand);
      if (sym != tableSymbols.end()) {
        int entAdr = symOperandForJmpAndCall(sym->second);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x92);
        tableSections[pendingSectionName].data.push_back((reg << 4) + 15);
        char d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr) >> 8);
        tableSections[pendingSectionName].data.push_back(d1);
        d1 = static_cast<char>(getDiscplacement(locationCounter + 4, entAdr));
        tableSections[pendingSectionName].data.push_back(d1);

        locationCounter += 4;

        // sad u registru imamo vrednost simbola (kad se razresi),
        // pa treba da uradimo jos jednu inst koja ce dohvatit vrednost iz memorije sa ove adrese u reg
        // proveri
        int code2 = (0b10010010 << 24);
        code2 = code2 | (reg << 20);
        code2 = code2 | (reg << 16);
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(code2 >> 24);
        tableSections[pendingSectionName].data.push_back(code2 >> 16);
        tableSections[pendingSectionName].data.push_back(code2 >> 8);
        tableSections[pendingSectionName].data.push_back(code2);
      }
    } else if (operand[0] == '%'){
      // operand je tipa %reg
      operand = operand.substr(1); // skidam %
      int reg2;
      if (operand == "sp") reg2 = 14;
      if (operand == "pc") reg2 = 15;
      else reg2 = stoi(operand.substr(1));
      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      tableSections[pendingSectionName].data.push_back(0x91);
      tableSections[pendingSectionName].data.push_back((reg << 4) + reg2);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x00);
    } else if (operand.size() <= 6){
      // operand je tipa [%reg]
      int reg2;
      string r = operand.substr(2,operand.size() - 3); // sklanjamo [% ]
      if (r == "sp") {
        reg2 = 14;
      } else if (r == "pc") {
         reg2 = 15;
      } else reg2 = stoi(r.substr(1));

      tableSections[pendingSectionName].offsets.push_back(locationCounter);
      tableSections[pendingSectionName].data.push_back(0x92);
      tableSections[pendingSectionName].data.push_back((reg << 4) + reg2);
      tableSections[pendingSectionName].data.push_back(0x00);
      tableSections[pendingSectionName].data.push_back(0x00);
    } else {
      // operand je ili [%reg + 123] ili [%reg + abc]
      const char delim = '+';
      vector<string> ops;
      tokenize(operand, delim, ops);
      string r = ops[1]; // r je ili "abc" ili "123"
      int reg2;
      // proveri [%r1 + 123]
      string r2 = ops[0].substr(2,ops[0].size() - 3); // r2 je rx ili sp ili pc
      if (r2 == "sp") {
        reg2 = 14;
      } else if (r2 == "pc"){
        reg2 = 15;
      } else {
        //cout << "ovaj je" << endl;
        reg2 = stoi(r2.substr(1));
        //cout << "okej" << endl;
      }

      if (isalpha(r[0]) != 0) exit(-1); // ne moze [%reg + abc] jer nakon asembliranja ne znamo vrednsot abc
      else {
        //r = r.substr(1); ipak je vec sklonjen // sklanjamo blanko karakter
        int lit = pretvoriUDecimalnuVrednost(r);
        if(lit < 0 || lit > 4095) exit(-1); // ni ovo nije validno jer ne moze da se smesti u instrukciju
        tableSections[pendingSectionName].offsets.push_back(locationCounter);
        tableSections[pendingSectionName].data.push_back(0x92);
        tableSections[pendingSectionName].data.push_back((reg << 4) + reg2);
        tableSections[pendingSectionName].data.push_back(lit >> 8);
        tableSections[pendingSectionName].data.push_back(lit);
      }
    }

    locationCounter +=4;
  }
  return true;
}

bool Assembler::second_pass(){
  locationCounter = 0;
  pendingSectionName = "";
  smatch sm;
  for (size_t i = 0 ; i < modified_lines.size(); i++){
    if (regex_search(modified_lines[i], sm, regex_for_label)){
      // nista ne radimo
    }
    else if (regex_search(modified_lines[i], sm, regex_for_global)){
      // proverimo da li su svi globalni simboli definisani
      //proveri
      string simboli = sm.str(1);
      const char delimiter = ',';
      vector<string> simbol;
      tokenize(simboli, delimiter, simbol);
      for (auto & s : simbol){
        map<string, EntrySymbolTable>::iterator sym = tableSymbols.find(s);
        if (sym != tableSymbols.end()){
          if (sym->second.is_defined == false) exit(-1);
        }
        else exit(-1);
      }

    }
    else if (regex_search(modified_lines[i], sm, regex_for_extern)){
      //nista ne radimo
    }
    else if (regex_search(modified_lines[i], sm, regex_for_section)){
      // proveri logiku za kacenje bazena literala i pisanje jmp-a na kraj sekcije
      skipTheLiteralPool();
      string secName = sm.str(1);
      locationCounter = 0;
      pendingSectionName = secName;
    }
    else if (checkIfSkipDirective2(modified_lines[i])){
      //ne mora ovde nista
    }
    else if (regex_search(modified_lines[i], sm, regex_for_end)){
      skipTheLiteralPool();
      Assembler::assembling_finished = 1;
      break;
    }
    else if (checkIfWordDirective2(modified_lines[i])){
      // ne mora nista ovde
    }
    else if (checkForInstructionInLine2(modified_lines[i])){

    }
  }
  return true;
}






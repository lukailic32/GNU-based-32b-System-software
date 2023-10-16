#include "../inc/linker.h"
#include <sstream>

string file_input;
string nextLineType;
int pendingSection = -1;
// mapa koja ce mi cuvati u kom fajlu, u kojoj njegovoj sekciji,
// na kojoj adresi se nalazi koji bajt
map <string, map<int, map <unsigned int, string>>> byteOnAddrInSectionInFile;
// opis sekcija; u kom fajlu i za koju sekciju se odnosi
map<string, map <string, sectionData>> sectionDescOfSectionInFile;
// opis simbola; koji simbol se nalazi u kojoj sekciji kog fajla
map<string, map<int, map<string, symbolData>>> symbolDescOfSymbolInSectionInFile;
// polje za realokaciju u sekciji u fajlu
map<string, map<int, list<relocationEntry>>> relocEntryInSectionInFile;
// simbol i njegova adresa u memoriji
map<string, unsigned int> symbolNameToAddress;

vector<orderElement> orderedSections;
map<unsigned int, string> addressToBytes;


int countDigits(int number){
  int count;
  while (number != 0){
    number /= 10;
    ++count;
  }
  // ako je broj 0 ima jednu cifru
  if (count == 0) count = 1;

  return count;
}

void splitWordsInRelocTable(const string& input, vector<std::string>& words) {
    istringstream iss(input);
    string word;

    while (getline(iss, word, ':')) {
        words.push_back(word);
    }
}

vector<string> splitWordsInSymbolTable(const string& input) {
    vector<string> words;
    istringstream iss(input);
    string word;

    while (iss >> word) {
        words.push_back(word);
    }

    return words;
}

void splitWordsInSectionTable(string& input, vector<string>& words) {
    istringstream iss(input);
    string word;

    while (getline(iss, word, ':')) {
        // Razdvajanje reči unutar sekvence razdvojene ":"
        istringstream wordStream(word);
        string subword;
        while (wordStream >> subword) {
            words.push_back(subword);
        }
    }
}

Linker::Linker(string ofn, vector<sectionWithAddress> &sio, vector<string> &fnio){
  hex_output_file = ofn;
  sectionsInOrder = sio;
  fileNamesInOrder = fnio;
}

void Linker::sortSectionsInOrder() {
  for (int i = 0; i < sectionsInOrder.size() - 1; ++i){
    for (int j = 0; j < sectionsInOrder.size() - i - 1; ++j){
      if (sectionsInOrder[j].address > sectionsInOrder[j+1].address) {
        swap(sectionsInOrder[j], sectionsInOrder[j+1]);
      }
    }
  }
}

void Linker::linkerInitialize(){
  // sortira sekcije koje su prosledjene kao argumenti
  // -place=imeSekcije@adresa
  sortSectionsInOrder();

}

void Linker::proccessCode(string line){
  int addr = stoi(line);
  // izracunamo broj cifara u adresi, preskocimo njih, : i razmak proveri
  string temp = line;
  temp.erase(0, countDigits(addr) + 2);
  byteOnAddrInSectionInFile[file_input][pendingSection][addr] = temp;
}

void Linker::proccessReloc(string line){
  vector<string> words;
  splitWordsInRelocTable(line, words);

  relocationEntry relData;
  relData.name = words.back();
  words.pop_back();
  relData.address = stoi(words.back());
  words.pop_back();
  relData.secId = stoi(words.back());
  words.pop_back();

  // na kraj liste realokacija za ovaj fajl i ovu sekciju
  relocEntryInSectionInFile[file_input][relData.secId].push_back(relData);

}

void Linker::proccessSymbol(string line){
  vector<string> words = splitWordsInSymbolTable(line);
  // 1. rec je broj sekcije, 2. rec je type, 3. rec je ime simbola, 4. rec je isDefined
  symbolData symData;
  symData.defined = stoi(words.back());
  words.pop_back();
  symData.address = stoi(words.back());
  words.pop_back();
  symData.name = words.back();
  words.pop_back();
  symData.type = words.back();
  words.pop_back();
  int secId = stoi(words.back());
  words.pop_back();

  if (symData.type == "global") {
    for (auto it = symbolDescOfSymbolInSectionInFile.begin(); it != symbolDescOfSymbolInSectionInFile.end(); it++){
      for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
        for (auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++){
          if (it3->first == symData.name && it3->second.type == "global"){
            cout << "Visestruka definicija simbola" << endl;
            exit(-2);
          }
        }
      }
    }
  }
  
  symbolDescOfSymbolInSectionInFile[file_input][secId][symData.name] = symData;
}

void Linker::proccessSection(string line){
  vector<string> words;
  splitWordsInSectionTable(line, words);
  sectionData secData;
  secData.size = stoi(words.back());
  words.pop_back();
  secData.sectionName = words.back();
  words.pop_back();
  secData.id = stoi(words.back());
  words.pop_back();
  
  //cout << secData.id << " " << secData.sectionName << " " << secData.size << endl;
  sectionDescOfSectionInFile[file_input][secData.sectionName] = secData; 
}

bool Linker::proccessLine(string line){
  if (nextLineType == "code"){
    proccessCode(line);
  } else if (nextLineType == "reloc"){
    proccessReloc(line);
  } else if (nextLineType == "symbol"){
    proccessSymbol(line);
  } else if (nextLineType == "section"){
    proccessSection(line);
  } else {
    cout << "GRESKA U CITANJU IZ ULAZNOG FAJLA" << endl;
    exit(-1);
  }
  return true;
}

bool Linker::linc(){
  for (int i = 0; i < fileNamesInOrder.size(); i++){
    file_input = fileNamesInOrder.front();
    //proveri
    fileNamesInOrder.erase(fileNamesInOrder.begin());
    fileNamesInOrder.push_back(file_input);

    ifstream inputFile(file_input);
    string line;

    while(getline(inputFile, line)){
      if (line.empty()) continue;
      if (line.substr(0, 8) == "section "){
        pendingSection = stoi(line.erase(0, 8));
        nextLineType = "code";
      }
      else if (line == "relocation_table:"){
        nextLineType = "reloc";
      }
      else if (line == "symbol_table:") {
        nextLineType = "symbol";
      }
      else if (line == "section_table:"){
        nextLineType = "section";
      } // PROVERI
      else if (!proccessLine(line)){
        inputFile.close();
        return false;
      }
    }
    inputFile.close();

  }
  return true;
}

bool Linker::sectionFromFileNotInList(string file, string section){
  for (auto it = orderedSections.begin(); it != orderedSections.end(); it++){
    if (it->fileName == file && it->sectionName == section) return false;
  }
  return true;
}

void Linker::checkForSectionsWithSameNameDiffrentFile(string file, string section){
  // prolazim kroz sve fajlove
  for (auto it = fileNamesInOrder.begin(); it != fileNamesInOrder.end(); it++){
    string currFile = *it;
    // jedino ne proveravamo za file u kom je sekcija cije ime trazimo
    if (currFile != file){
      // ako u ovom fajlu nadje sekciju sa ovim imenom neka je nakaci
      if (sectionDescOfSectionInFile[currFile].find(section) != sectionDescOfSectionInFile[currFile].end()){
        orderElement oe;
        oe.fileName = currFile;
        oe.sectionName = section;
        oe.sectionIndex = sectionDescOfSectionInFile[currFile][section].id;
        if (orderedSections.empty()) oe.startAddr = 0; // nikad se nece desiti
        else oe.startAddr = orderedSections.back().endAddr;
        oe.endAddr = oe.startAddr + sectionDescOfSectionInFile[currFile][section].size;
        if (oe.endAddr > 0xFFFFFFFF) {
          cout << "Ne moze sve da stane u memoriju na trazene adrese" <<endl;
        }
        orderedSections.push_back(oe);        
      }
    }
  }
}

void Linker::orderSections(){
  
  // PRVO RASPORED SEKCIJA SA PREDEFINISANIM ADRESAMA ZA SMESTANJA

  //prolazimo kroz sve sekcije koje su nam prosledjene kao ulazni paramteri
  for (auto it = sectionsInOrder.begin(); it != sectionsInOrder.end(); it++){
    string sectionName = it->section;
    unsigned int baseAddr = it->address;
    // ako je prethodna sekcija toliko velika da sledeca kojoj smo dodeli pocetnu adresu upada u nju, bacamo gresku
    if (!orderedSections.empty()){
      if (baseAddr < orderedSections.back().endAddr){
        // NIJE MI BAS NAJJASNIJE
        cout << "GRESKA: Preklapanje sekcija" << endl;
        exit(-1);
      }
    }
    //prolazimo kroz svaki fajl
    for  (auto it2 = fileNamesInOrder.begin(); it2 != fileNamesInOrder.end(); it2++){
      //pristupamo sekcijama odredjenog fajla
      string file = *it2;
      if (sectionDescOfSectionInFile.find(file) != sectionDescOfSectionInFile.end()){
        // i trazimo u njemu sekciju sa istim imenom, zbog obilaska svih fajlova, imacemo spojene sve sekcije sa istim imenom iz razlicitih fajlova
        if (sectionDescOfSectionInFile[file].find(sectionName) != sectionDescOfSectionInFile[file].end()){
          orderElement oe;
          oe.fileName = (file);
          oe.sectionName = sectionName;
          oe.sectionIndex = sectionDescOfSectionInFile[file][sectionName].id;
          oe.startAddr = baseAddr;
          oe.endAddr = oe.startAddr + sectionDescOfSectionInFile[file][sectionName].size;
          if (oe.endAddr > 0xFFFFFFFF || oe.endAddr < 0 || oe.startAddr < 0){
            cout << "IZVAN MEMORIJE" << endl;
            exit(-1);
          }
          baseAddr = oe.endAddr;
          orderedSections.push_back(oe);
        }
      }
    }
  }

  // OVDE PRAVIM RASPORED SVIH DRUGIH SEKCIJA TAKO STO IH STAVLJAM NAKON OVIH DO SAD

  for (auto it = fileNamesInOrder.begin(); it != fileNamesInOrder.end(); it ++){
    string currFile = *it;
    for (auto it2 = sectionDescOfSectionInFile[currFile].begin(); it2 != sectionDescOfSectionInFile[currFile].end(); it2++){
      string section = it2->first;
      if (sectionFromFileNotInList(currFile, section)){
        orderElement oe;
        oe.fileName = currFile;
        oe.sectionName = section;
        oe.sectionIndex = sectionDescOfSectionInFile[currFile][section].id;
        if (orderedSections.empty()) oe.startAddr = 0;
        else oe.startAddr = orderedSections.back().endAddr;
        oe.endAddr = oe.startAddr + sectionDescOfSectionInFile[currFile][section].size;
        if (oe.endAddr > 0xFFFFFFFF) {
          cout << "Ne moze sve da stane u memoriju na trazene adrese" <<endl;
        }
        orderedSections.push_back(oe);
        //proveri
        checkForSectionsWithSameNameDiffrentFile(currFile, section);
      }
    }
  }

}

void Linker::adjustSymbolTable(){
  // prolazim kroz sekcije koje su rasporedjene u memoriju
  for (auto it = orderedSections.begin(); it != orderedSections.end(); it++){
    string currFile = it->fileName;
    int secId = it->sectionIndex;
    // prolazim kroz sve simbole izabrane sekcije u fajlu i azuriramo adrese njenih simbola
    for (auto it2 = symbolDescOfSymbolInSectionInFile[currFile][secId].begin();
    it2 != symbolDescOfSymbolInSectionInFile[currFile][secId].end(); it2++)
    {
      it2->second.address += it->startAddr;
    }
  }
}

void Linker::adjustRelocationTable(){
  for (auto it = orderedSections.begin(); it != orderedSections.end(); it++){
    string currFile = it->fileName;
    int secId = it->sectionIndex;
    for (auto it2 = relocEntryInSectionInFile[currFile][secId].begin(); 
    it2 != relocEntryInSectionInFile[currFile][secId].end(); it2++)
    {
      it2->address += it->startAddr;
    }
  }
}

void Linker::getDefinedSymbols() {
  for(auto it = symbolDescOfSymbolInSectionInFile.begin(); it != symbolDescOfSymbolInSectionInFile.end(); it++){
    for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
      for(auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++){
        if(it3->second.defined && it3->second.type == "global") { 
          symbolNameToAddress[it3->first] = it3->second.address;
        }
      }
    }
  }
}

void Linker::resolveSymbols(){
  for (auto it = symbolDescOfSymbolInSectionInFile.begin(); it != symbolDescOfSymbolInSectionInFile.end(); it++){
    for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
      for (auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++){
        if (it3->second.type == "extern" && !(it3->second.defined)){
          if (symbolNameToAddress.find(it3->first) != symbolNameToAddress.end()) {
            //proveri
            it3->second.defined = 1;
          }
        }
      }
    }
  }
}

void Linker::verifyAllDefined(){
  for(auto it = symbolDescOfSymbolInSectionInFile.begin(); it != symbolDescOfSymbolInSectionInFile.end(); it++){
    for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
      for(auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++){
        if(!(it3->second.defined)){
          cout << "GRESKA, SYMBOL " << it3->first << " NIJE DEFINISAN\n";
          exit(-1);
        }
      }
    }
  }
}

void Linker::createOutput(){
  for (auto it = orderedSections.begin(); it != orderedSections.end(); it++){
    for (auto it2 = byteOnAddrInSectionInFile[it->fileName][it->sectionIndex].begin();
    it2 != byteOnAddrInSectionInFile[it->fileName][it->sectionIndex].end(); it2++){
      // stvarnu adresu za svaki bajt nalazimo tako sto saberemo
      // njegov offset u sekciji i pocetnu adresu sekcije
      unsigned int realAddr = it2->first + it->startAddr;
      string byte = it2->second;
      for (auto it3 = relocEntryInSectionInFile[it->fileName][it->sectionIndex].begin();
      it3 != relocEntryInSectionInFile[it->fileName][it->sectionIndex].end(); it3++){
        // ako se adresa ovog simobla nalazi u relokacionom zapisu 
        // onda treba da razresimo tu realokaciju
        if (it3->address == realAddr){
          if (symbolNameToAddress.find(it3->name) != symbolNameToAddress.end()){
            // simbol je globalan, dohvatamo ga iz ovih simbola za razresavanje
            byte = translateToHexLittleEndian(symbolNameToAddress[it3->name]);
          } else {
            // simbol je lokalan
            byte = translateToHexLittleEndian(symbolDescOfSymbolInSectionInFile[it->fileName][it->sectionIndex][it3->name].address);
          }
          break;
        }
      }
      addressToBytes[realAddr] = byte;
    }
  }
}

void Linker::writeOutputFile(){
  ofstream output_file(hex_output_file);
  unsigned int previousAddr = 0;
  int i = 0;

  for (auto it = addressToBytes.begin(); it != addressToBytes.end(); it++){
    string line = it->second;
    if (i % 2 == 0) {
      output_file << translateToHexExtended(it->first) << ": " << line.substr(0,2) << " ";
      output_file << line.substr(2,2) << " " << line.substr(4,2) << " " << line.substr(6,2) <<" ";
    } else {
      if (it->first == (previousAddr + 4)){
        output_file << line.substr(0,2) << " " <<line.substr(2,2) << " ";
        output_file << line.substr(4,2) << " " << line.substr(6,2) <<"\n";
      } else {
        output_file <<"\n";
        output_file << translateToHexExtended(it->first) << ": " << line.substr(0,2) << " ";
        output_file << line.substr(2,2) << " " << line.substr(4,2) << " " << line.substr(6,2) <<" ";
        i++;
      }
    }
    previousAddr = it->first;
    i++;
  }
}

void Linker::generateOutput(){
  createOutput();
  writeOutputFile();
}

void Linker::generateMemoryAndConnectTables(){
  orderSections();
  adjustSymbolTable();
  adjustRelocationTable();
  getDefinedSymbols();
  resolveSymbols();
  verifyAllDefined();
}
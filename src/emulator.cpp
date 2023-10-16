#include "../inc/emulator.h"
#include <vector>
#include <sstream>

#include <iomanip>

unsigned int gpr[16];
unsigned int csr[3];
unsigned char A_B;
unsigned char C_DH;
unsigned char D_D;
int regA;
int regB;
int regC;
int displ;

void tokenize(string const &str, const char delim, vector<string> &out)
{
    stringstream ss(str);
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

Emulator::Emulator(string file) {
  input_hex_file = file;
  gpr[15] = 0x40000000;
  for (int i = 0; i <= 14; i++) gpr[i] = 0;
  csr[0] = csr[1] = csr[2] = 0;
}

void Emulator::readRegs(unsigned int instruction){
  A_B = (instruction >> 16) & 0xFF;
  C_DH = (instruction >> 8) & 0xFF;
  D_D = instruction & 0xFF;

  regA = A_B >> 4;
  regB = A_B & 0xF;
  regC = C_DH >> 4;
  displ = ((C_DH & 0xF) << 8) + D_D;
  if(displ & 0x800) displ = displ | 0xFFFFF000;
}

void Emulator::pushPC(){
  memory[gpr[14]] = gpr[15];
}

void Emulator::decSP(){
  gpr[14] = gpr[14] - 4;
}

void Emulator::pushStatus(){
  memory[gpr[14]] = csr[0];
}

void Emulator::incPC(){
  gpr[15] += 4;
}

void Emulator::maskInterrupts(){
  csr[0] = csr[0] & (~0x1);
}

void Emulator::writeOutput(){
  cout << "---------------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction!" << endl;
  cout << "Emulated processor state:" << endl;

  for (int i = 0; i < 16; i++) {
    ostringstream oss;
    oss << "r" << i << "=0x";
    cout << oss.str() << translateToHexExtended(gpr[i]);
    if (i % 4 == 3) cout << endl;
    else cout << "\t"; 
  }

}

void Emulator::emulate(){
  /*
  GLAVNA METODA ZA OBRADU
  */
  bool halt = false;
  while (!halt){
    unsigned int instruction = memory[gpr[15]];
    unsigned char OC_MOD = instruction >> 24;
    incPC();
    if (OC_MOD == 0x00) {
      // INSTRUCTION HALT
      
      halt = true;
      //gpr[15] += 4; // procita ceo halt
      continue;
    }
    if (OC_MOD == 0x10) {
      // INSTUCTION INT
     // PRIPAZI DA PROCITAS 4B SVAKI PUT
      decSP();
      pushPC();
      decSP();
      pushStatus();
      csr[2] = 4;
      maskInterrupts();
      gpr[15] = csr[1];
      //cout << csr[1] << endl;
      continue;
    }
    /*            CALL           */
    if (OC_MOD == 0x20){
      // CALL REL NIKAD JE NE IZVRSAVAM
      readRegs(instruction);
      decSP();
      pushPC();
      gpr[15] = gpr[regA] + gpr[regB] + displ;
      continue;
    }
    if (OC_MOD == 0x21){
      // CALL ABS 
     // imam smestene info u regA, regB, regC i displ
      decSP();
      pushPC();
      readRegs(instruction);
      gpr[15] = memory[gpr[regA] + gpr[regB] + displ];
      continue;
    }


    /*          INSTRUKCIJE SKOKA           */
    if (OC_MOD == 0x30) {
      // JMP REL
      readRegs(instruction);
      gpr[15] = gpr[regA] + displ;
      continue;
    }
    if (OC_MOD == 0x31) {
      // BEQ REL
      readRegs(instruction);
      if (gpr[regB] == gpr[regC]) gpr[15] = gpr[regA] + displ;
      continue;
    }
    if (OC_MOD == 0x32) {
      // BNE REL
      readRegs(instruction);
      if (gpr[regB] != gpr[regC]) gpr[15] = gpr[regA] + displ;
      continue;
    }
    if (OC_MOD == 0x33) {
      // BGT REL
      readRegs(instruction);
      if (gpr[regB] > gpr[regC]) gpr[15] = gpr[regA] + displ;
      continue;
    }
    if (OC_MOD == 0x38) {
      // JMP ABS
      readRegs(instruction);
      gpr[15] = memory[gpr[regA] + displ];
      continue;
    }
    if (OC_MOD == 0x39) {
      // BEQ ABS
      readRegs(instruction);
      if (gpr[regB] == gpr[regC]) gpr[15] = memory[gpr[regA] + displ];
      continue;
    }
    if (OC_MOD == 0x3A) {
      // BNE ABS
      readRegs(instruction);
      if (gpr[regB] != gpr[regC]) gpr[15] = memory[gpr[regA] + displ];
      continue;
    }
    if (OC_MOD == 0x3B) {
      // BGT ABS
      readRegs(instruction);
      if (gpr[regB] > gpr[regC]) gpr[15] = memory[gpr[regA] + displ];
    }


    if (OC_MOD == 0x40) {
      // XCHG
      readRegs(instruction);
      int temp = gpr[regB];
      gpr[regB] = gpr[regC];
      gpr[regC] = temp;
      continue;
    }


    /*          ARITHMETIC (ADD, SUB, MUL, DIV)       */
    if (OC_MOD == 0x50) {
      // ADD
      cout << "Instrukcija ADD: " << instruction << endl;
      readRegs(instruction);
      gpr[regA] = gpr[regB] + gpr[regC];
      continue;
    }
    if (OC_MOD == 0x51) {
      // SUB
      cout << "Instrukcija SUB: " << instruction << endl;
      readRegs(instruction);
      gpr[regA] = gpr[regB] - gpr[regC];
      continue;
    }
    if (OC_MOD == 0x52) {
      // MULL
      cout << "Instrukcija MUL: " << instruction << endl;
      readRegs(instruction);
      gpr[regA] = gpr[regB] * gpr[regC];
      continue;
    }
    if (OC_MOD == 0x53) {
      // DIV
      cout << "Instrukcija DIV: " << instruction << endl;
      readRegs(instruction);
      gpr[regA] = gpr[regB] / gpr[regC];
      continue;
    }


    /*        LOGICAL (NOT, AND, OR, XOR)         */
    if (OC_MOD == 0x60) {
      // NOT
      readRegs(instruction);
      gpr[regA] = ~(gpr[regB]);
      continue;
    }
    if (OC_MOD == 0x61) {
      // AND
      readRegs(instruction);
      gpr[regA] = (gpr[regB] & gpr[regC]);
      continue;
    }
    if (OC_MOD == 0x62) {
      //OR
      readRegs(instruction);
      gpr[regA] = (gpr[regB] | gpr[regC]);
      continue;
    }
    if (OC_MOD == 0x63) {
      //XOR
      readRegs(instruction);
      gpr[regA] = (gpr[regB] ^ gpr[regC]);
      continue;
    }
    

    /*        SHIFT (SHL, SHR)          */
    if (OC_MOD == 0x70) {
      // SHL
      readRegs(instruction);
      gpr[regA] = gpr[regB] << gpr[regC];
      continue;
    }
    if (OC_MOD == 0x71) {
      // SHR
      readRegs(instruction);
      gpr[regA] = gpr[regB] >> gpr[regC];
      continue;
    }


    /*         ST DIR, PUSH, ST INDIR         */
    if (OC_MOD == 0x80) {
      // ST DIR
      readRegs(instruction);
      memory[gpr[regA] + gpr[regB] + displ] = gpr[regC];
      continue;
    }
    if (OC_MOD == 0x81) {
      readRegs(instruction);
      gpr[regA] = gpr[regA] + displ;
      memory[gpr[regA]] = gpr[regC];
      continue;
    }
    if (OC_MOD == 0x82) {
      // ST INDIR
      readRegs(instruction);
      memory[memory[gpr[regA] + gpr[regB] + displ]] = gpr[regC];
      continue;
    }
    

    /*          ISNTRUCTIONS FOR LOADING           */
    if (OC_MOD == 0x90) {
      // CSRRD
      cout << "Instrukcija ADD: " << instruction << endl;
      readRegs(instruction);
      gpr[regA] = csr[regB];
      continue;
    }
    if (OC_MOD == 0x91) {
      // LD REG1, REG2 ili 
      // LD LITEREAL, REG
      readRegs(instruction);
      gpr[regA] = gpr[regB] + displ;
      continue;
    }
    if (OC_MOD == 0x92) {
      readRegs(instruction);
      //cout << "lepo dohvatio iz memorije" << endl;
      // LD $SYMBOL, REG           ---> reg = symbol ( simbol se uzima iz memorije jer koritstimo bazen literala)
      // LD $LITERA(>=0x800), REG  ---> reg = literal ( literal se uzima iz memorije jer koristimo bazen literlala)
      // LD LITERAL(>=0x800), REG  ---> iz memorije na koju ukazuje literal uzimamo pravi literal -> reg = mem[mem[literal]]
      // LD LITERAL(<=0x800), REG  ---> iz memorije na koju ukazuje literal -> reg = mem[literal]
      // LD SYMBOL, REG            ---> reg = mem[symbol]
      // LD [%REG1], REG           ---> reg = mem[reg1]
      // LD [%REG1 + LITERAL], REG ---> reg = mem[reg1 + literal] (lit mora biti <12b da bi stao u instrukciju i da bi se znala njegova vrednost nakon asembl)
      gpr[regA] = memory[gpr[regB] + gpr[regC] + displ];
      //cout << gpr[regA];
      continue;
    }
    if (OC_MOD == 0x93) {
      // POP, RET ili drugi deo IRET-a(POP PC) (prvi je CSRWR sa steka)
      readRegs(instruction);
      gpr[regA] = memory[gpr[regB]];
      gpr[regB] = gpr[regB] + displ;
      continue;
    }
    if (OC_MOD == 0x94) {
      // CSRWR REG, CSR
      readRegs(instruction);
      csr[regA] = gpr[regB];
      continue;
    }
    if (OC_MOD == 0x95) {
      // NEMAM INSTRUKCIJU OVOG TIPA (DA CSR UZIMA VREDNOST DRUGOG CSR-a)
      readRegs(instruction);
      csr[regA] = csr[regB] | displ;
      continue;
    }
    if (OC_MOD == 0x96) {
      // NEMAM NI OVU (DA CSR UZIMA NEKU VREDNOST IZ MEM)
      readRegs(instruction);
      csr[regA] = memory[gpr[regB] + gpr[regC] + displ];
      continue;
    }
    if (OC_MOD == 0x97) {
      // IRET PRVI DEO, SKIDA STATUS SA STEKA
      readRegs(instruction);
      csr[regA] = memory[gpr[regB]];
      gpr[regB] = gpr[regB] + displ;
      continue;
    }
  }
  writeOutput();
}

void Emulator::readHexFile(){
  /*
  METODA ZA INICIJALIZACIJU memorije
  */

  input_file.open(input_hex_file);
  if (!input_file.is_open()) {
    cout << "Greska prilikom otvaranja ulaznog fajla" << endl;
    exit(-1);
  }

  string currentLine = "";
  string restOfLine = "";
  int address;
  while(getline(input_file, currentLine)){
    if (currentLine.empty()) continue;
    string addr = currentLine.substr(0,8);
    address = stol(addr, nullptr, 16);
    restOfLine = currentLine.substr(10);

    vector<string> bytes;
    tokenize(restOfLine, ' ', bytes);
    unsigned int value1;
    unsigned int value2;

    if (bytes.size() < 6){
      value1 = 0;
      value2 = 0;
      // ima samo 4 bajta u redu (kraj sekcije)
        // jer je little endian
        unsigned int first = stoul(bytes.back(), NULL, 16);
        bytes.pop_back();
        unsigned int second = stoul(bytes.back(), NULL, 16);
        bytes.pop_back();
        unsigned int third = stoul(bytes.back(), NULL, 16);
        bytes.pop_back();
        unsigned int fourth = stoul(bytes.back(), NULL, 16);
        bytes.pop_back();
        value1 = (first << 24) | (second << 16) | (third << 8) | fourth;
        memory[address] = value1;
    } else {
      value1 = 0;
      value2 = 0;
      // ima 8 bajtova
      // citam prvo prva 4 i to od pozadi
      unsigned int fifth = stoul(bytes.back(), NULL, 16);
      bytes.pop_back();
      unsigned int sixth = stoul(bytes.back(), NULL, 16);
      bytes.pop_back();
      unsigned int seventh = stoul(bytes.back(), NULL, 16);
      bytes.pop_back();
      unsigned int eighth = stoul(bytes.back(), NULL, 16);
      bytes.pop_back();
      value2 = (fifth << 24) | (sixth << 16) | (seventh << 8) | eighth;
    // citam druga 4 bajta
      unsigned int first = stoul(bytes.back(), NULL, 16);
      bytes.pop_back();
      unsigned int second = stoul(bytes.back(), NULL, 16);
      bytes.pop_back();
      unsigned int third = stoul(bytes.back(), NULL, 16);
      bytes.pop_back();
      unsigned int fourth = stoul(bytes.back(), NULL, 16);
      bytes.pop_back();
      value1 = (first << 24) | (second << 16) | (third << 8) | fourth;      

      memory[address] = value1;
      memory[address+4] = value2;
    }
  }
  cout << "Velicina memorije" << memory.size() << endl;
  // imamo ucitan hex fajl u memoriju
  // sad ga treba emulirati
}
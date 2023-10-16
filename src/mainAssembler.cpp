#include <regex>
#include "../inc/assembler.h"

using namespace std;

int main(int argc, const char* argv[]){
  string ulazniFile;
  string izlazniFile;
  if (argc < 2){
    cout << "Niste uneli dovoljno argumenata" << endl;
    return -1;
  }

  string o = argv[1];
  if (o != "-o"){
    cout << "Asembler kao drugi argument zahteva -o" << endl;
    return -1;
  } else {
    //cout << "USPESNO UNETI ARGUMENTI" << endl;
    izlazniFile = argv[2];
    ulazniFile = argv[3];
    Assembler as(ulazniFile, izlazniFile);
    int ret = as.checkFileExtensions();
    if (ret < 0) {
      cout << "Lose proslednjeni parametri" << endl;
      return -1;
    }
    as.initializeAssembler();
    ret = as.assembly();
    if (ret < 0){
      cout << "Greska pri radu asemblera" << endl;
      return -1;
    }
    return 0;
  }
}
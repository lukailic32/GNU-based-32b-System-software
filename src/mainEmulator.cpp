#include "../inc/emulator.h"

int main(int argc, char* argv[]){
  string input_file;
  if (argc < 2) {
    cout << "Pogresno ste pokrenuli emulator." << endl;
    return -1;
  }else {
    if (argc == 2){
      input_file = argv[1];

      Emulator em(input_file);
      em.readHexFile();
      em.emulate();
    }
  }
}
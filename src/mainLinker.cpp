#include "../inc/linker.h"
#include "../inc/regex.h"

using namespace std;



int main(int argc, char **argv){
  
  bool output_file = false;
  string output_file_name = "linker_out_generic";
  bool place_file = false;
  regex place_regex("-place=([_a-zA-Z][a-zA-Z_0-9]*)@(0[xX][0-9afA-F]+)$");
  smatch sm;
  bool hex_output = false;
  vector<sectionWithAddress> sectionsInOrder;
  vector<string> fileNamesInOrder;

  for (int i = 1; i < argc; i++){
    string pendingArg = argv[i];

    if (pendingArg == "-o"){
      output_file = true;
    }
    else if (pendingArg == "-hex") {
      hex_output = true;
    } else if (regex_search(pendingArg, sm, place_regex)){
      place_file = true;
      sectionWithAddress str;
      str.section = sm.str(1);
      //cout << sm.str(2) << endl;
      str.address = stol(sm.str(2), nullptr, 16);
      sectionsInOrder.push_back(str);
    } 
    else if (output_file){
      output_file_name = pendingArg;
      output_file = false;
    }
    else {
      fileNamesInOrder.push_back(pendingArg);
    }
    
  }

  Linker li(output_file_name, sectionsInOrder, fileNamesInOrder);
  li.linkerInitialize();
  bool ret = li.linc();
  if (!ret) {
    cout << "Greska pri radu linkera" << endl;
  }
  li.generateMemoryAndConnectTables();
  li.generateOutput();
}
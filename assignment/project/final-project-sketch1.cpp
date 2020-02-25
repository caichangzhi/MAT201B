#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp" 
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp" 

#include <fstream>
#include <vector>

using namespace al;
using namespace std;

string slurp(string fileName) {
  fstream file(fileName);
  string returnValue = "";
  while (file.good()) {
    string line;
    getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}

int main() { AlloApp().start();}

//This is file may be useless

#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace al;

Parameter moveRate{"/moveRate", "", 1.0, "", 0.0, 2.0};
Parameter turnRate{"/turnRate", "", 1.0, "", 0.0, 2.0};
Parameter localRadius{"/localRadius", "", 0.4, "", 0.01, 0.9};
Parameter size{"/size", "", 1.0, "", 0.0, 2.0};
Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
ControlGUI gui;

void guiInit(){
    gui << moveRate << turnRate << localRadius << size << ratio;
    gui.init();
}
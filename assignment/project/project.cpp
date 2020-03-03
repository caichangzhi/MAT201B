#include "al/ui/al_ControlGUI.hpp"
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"
#include "al/graphics/al_Font.hpp"
#include "birds.hpp"
#include "predators.hpp"
#include "sound.cpp"
#include "gui.hpp"

#include <iostream>
#include <fstream>
#include <vector>

using namespace al;
using namespace std;

string slurp(string fileName); 

struct SharedState{
  BirdsAttribute birds[birdsN];
  PredatorsAttribute predators[predatorsN];
  float size;
  float ratio;
};

class MyApp : public DistributedAppWithState<SharedState> {
  Font font;
  Mesh text;

  Parameter moveRate{"/moveRate", "", 1.0, "", 0.0, 2.0};
  Parameter turnRate{"/turnRate", "", 1.0, "", 0.0, 2.0};
  Parameter localRadius{"/localRadius", "", 0.4, "", 0.01, 0.9};
  Parameter size{"/size", "", 1.0, "", 0.0, 2.0};
  Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  void onCreate() override {
    imguiInit();

    font.load("data/VeraMono.ttf", 28, 1024);
    font.alignCenter();

    cuttleboneDomain =
        CuttleboneStateSimulationDomain<SharedState>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }

    // gui << moveRate << turnRate << localRadius << size << ratio;
    // gui.init();
    guiInit();
    navControl().useMouse(false);

    birdsShader.compile(slurp("../tetrahedron-vertex.glsl"),
                       slurp("../tetrahedron-fragment.glsl"),
                       slurp("../tetrahedron-geometry.glsl"));
    predatorsShader.compile(slurp("../tetrahedron-vertex1.glsl"),
                       slurp("../tetrahedron-fragment1.glsl"),
                       slurp("../tetrahedron-geometry1.glsl"));

    birdsMesh.primitive(Mesh::POINTS);
    predatorsMesh.primitive(Mesh::POINTS);

    nav().pos(0, 0, 10);
  }

  void onAnimate(double dt) override {
    Vec3f steer(0, 0, 0);
    Vec3f diff(0, 0, 0);
    if (cuttleboneDomain->isSender()) {

    for (unsigned i = 0; i < birdsN; i++) {
      for (unsigned j = 1 + i; j < birdsN; j++) {
        float distance = (birds[j].pos() - birds[i].pos()).mag();
        if (distance < localRadius) {
          diff = birds[j].pos() - birds[i].pos();
          diff.normalize();
          steer = steer + diff;
        }
      }
      steer.normalize();
      birds[i].faceToward(-steer);
      steer = (0, 0, 0);
    }

    for (unsigned i = 0; i < predatorsN; i++){
      for (unsigned j = 0; j < birdsN; j++){
        float distance = (predators[i].pos() - birds[j].pos()).mag();
        if(distance < 0.1){
          //dt = turnRate;
          //TODO: erase the birds which is eaten and generate new one
          //TODO: make the predator rotate several times to notify it find birds

          font.write(text, "Predators are eating birds", 0.2f);
          //font.write(text, "Predator i are eating bird j", 0.2f);
          //TODO: add the specific number of which predator eat which bird
          //e.g. "predator 3 are eating bird 80"
        }
        else{
        font.write(text, "Predators are searching birds", 0.2f);
        }
      }
    }

    for (unsigned i = 0; i < birdsN; i++) {
      birds[i].pos() += birds[i].uf() * moveRate * 0.002;
    }

    for (unsigned i = 0; i < predatorsN; i++) {
      predators[i].pos() += predators[i].uf() * moveRate * 0.002;
    }

    for (unsigned i = 0; i < birdsN; i++) {
      if (birds[i].pos().mag() > 1.1) {
        birds[i].pos(rv());
        birds[i].faceToward(rv());
      }
    }

    for (unsigned i = 0; i < predatorsN; i++) {
      if (predators[i].pos().mag() > 1.1) {
        predators[i].pos(rv());
        predators[i].faceToward(rv());
      }
    }

    for (unsigned i = 0; i < birdsN; i++) { 
        BirdsAttribute b;
        b.position = birds[i].pos();
        b.forward = birds[i].uf();
        b.up = birds[i].uu();
        state().birds[i] = b;
      }

    for (unsigned i = 0; i < predatorsN; i++) { 
        PredatorsAttribute p;
        p.position = predators[i].pos();
        p.forward = predators[i].uf();
        p.up = predators[i].uu();
        state().predators[i] = p;
      }
      state().size = size.get();
      state().ratio = ratio.get();
    }
    else{ }

    vector<Vec3f>& v(birdsMesh.vertices());
    vector<Vec3f>& n(birdsMesh.normals());
    vector<Color>& c(birdsMesh.colors());
    for (unsigned i = 0; i < birdsN; i++) {
      v[i] = state().birds[i].position;
      n[i] = state().birds[i].forward;
      const Vec3d& up(state().birds[i].up);
      c[i].set(up.x, up.y, up.z);
    }

    vector<Vec3f>& pv(predatorsMesh.vertices());
    vector<Vec3f>& pn(predatorsMesh.normals());
    vector<Color>& pc(predatorsMesh.colors());
    for (unsigned i = 0; i < predatorsN; i++) {
      pv[i] = state().predators[i].position;
      pn[i] = state().predators[i].forward;
      const Vec3d& up(state().predators[i].up);
      pc[i].set(up.x, up.y, up.z);
    }
  }

  void onDraw(Graphics& g) override {
    g.clear(0.1, 0.1, 0.1);
    gl::depthTesting(true); 
    gl::blending(true); 
    gl::blendTrans(); 
    g.shader(birdsShader);
    g.shader(predatorsShader);
    g.shader().uniform("size", state().size * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(birdsMesh);
    g.draw(predatorsMesh);
    g.texture();
    font.tex.bind();
    g.draw(text);
    font.tex.unbind();

    if (isPrimary()) {
      gui.draw(g);
    }
  }
};

int main() {
  MyApp app;
  Appp sound;
  app.start();
  sound.start();
}

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

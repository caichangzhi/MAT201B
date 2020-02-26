#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/sound/al_SoundFile.hpp"

using namespace al;

#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

const int birdsN = 10;
const int predatorsN = 10;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

string slurp(string fileName); 

struct Birds : Pose {
  Vec3f heading;
  Vec3f center; 
  unsigned flockCount{1};
};

struct Predators : Pose {
  Vec3f heading;
  Vec3f center; 
  unsigned flockCount{1};
};

Birds birds[birdsN];
Predators predators[predatorsN];

struct BirdsAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;
};

struct PredatorsAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;
};

struct SharedState{
  BirdsAttribute birds[birdsN];
  PredatorsAttribute predators[predatorsN];
  float size;
  float ratio;
};

//TODO: Make another control GUI to control predators' moverate (not necessary)
class MyApp : public DistributedAppWithState<SharedState> {
  SoundFilePlayerTS playerTS;
  std::vector<float> buffer;
  bool loop = true;
  Font font;
  Parameter moveRate{"/moveRate", "", 1.0, "", 0.0, 2.0};
  Parameter turnRate{"/turnRate", "", 1.0, "", 0.0, 2.0};
  Parameter localRadius{"/localRadius", "", 0.4, "", 0.01, 0.9};
  Parameter size{"/size", "", 1.0, "", 0.0, 2.0};
  Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  ShaderProgram birdsShader;
  ShaderProgram predatorsShader;
  Mesh birdsMesh;
  Mesh predatorsMesh;
  Mesh text;

  void onInit() override {
    const char name[] = "data/fly.wav";
    if (!playerTS.open(name)) {
      std::cerr << "File not found: " << name << std::endl;
      quit();
    }
    std::cout << "sampleRate: " << playerTS.soundFile.sampleRate << std::endl;
    std::cout << "channels: " << playerTS.soundFile.channels << std::endl;
    std::cout << "frameCount: " << playerTS.soundFile.frameCount << std::endl;
    playerTS.setLoop();
    playerTS.setPlay();
  }

  void onCreate() override {
    imguiInit();

    font.load("data/VeraMono.ttf", 28, 1024);
    font.alignCenter();
    // font.write(text, "hello font", 0.2f);

    cuttleboneDomain =
        CuttleboneStateSimulationDomain<SharedState>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }

    gui << moveRate << turnRate << localRadius << size << ratio;
    gui.init();
    navControl().useMouse(false);

    //TODO: find how make the difference of the color between the birds and predators
    //e.g: make the predators look white and birds look colorful 
    birdsShader.compile(slurp("../tetrahedron-vertex.glsl"),
                       slurp("../tetrahedron-fragment.glsl"),
                       slurp("../tetrahedron-geometry.glsl"));
    predatorsShader.compile(slurp("../tetrahedron-vertex1.glsl"),
                       slurp("../tetrahedron-fragment1.glsl"),
                       slurp("../tetrahedron-geometry1.glsl"));

    birdsMesh.primitive(Mesh::POINTS);
    predatorsMesh.primitive(Mesh::POINTS);

    for (int _ = 0; _ < birdsN; _++) {
      Birds b;
      b.pos(rv());
      b.faceToward(rv());
      birds[_] = b;
      birdsMesh.vertex(b.pos());
      birdsMesh.normal(b.uf());
      const Vec3f& up(b.uu());
      birdsMesh.color(up.x, up.y, up.z);
    }

    for (int _ = 0; _ < predatorsN; _++) {
      Predators p;
      p.pos(rv());
      p.faceToward(rv());
      predators[_] = p;
      predatorsMesh.vertex(p.pos());
      predatorsMesh.normal(p.uf());
      const Vec3f& up(p.uu());
      predatorsMesh.color(up.x, up.y, up.z);
    }

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
          dt = turnRate;
          //TODO: erase the birds which is eaten and generate new one
          //TODO: make the predator rotate several times to notify it find birds

          font.write(text, "Predators are eating birds", 0.2f);
          //TODO: add the specific number of which predator eat which bird
          //e.g. "predator 3 are eating bird 80"
        }
        // else{
        // font.write(text, "Predators are searching birds", 0.2f);
        // }
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

  //TODO: when predators eat birds, play 'eat.wav', other time play 'fly.wav'
  void onSound(AudioIOData& io) override {
    int frames = (int)io.framesPerBuffer();
    int channels = playerTS.soundFile.channels;
    int bufferLength = frames * channels;
    if ((int)buffer.size() < bufferLength) {
      buffer.resize(bufferLength);
    }
    playerTS.getFrames(frames, buffer.data(), (int)buffer.size());
    int second = (channels < 2) ? 0 : 1;
    while (io()) {
      int frame = (int)io.frame();
      int idx = frame * channels;
      io.out(0) = buffer[idx];
      io.out(1) = buffer[idx + second];
    }
  }

  void onExit() override { imguiShutdown(); }
};

int main() {
  MyApp app;
  app.configureAudio(44100, 512, 2, 0);
  app.start();
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
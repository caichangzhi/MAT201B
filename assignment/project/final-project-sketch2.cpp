#include "al/app/al_DistributedApp.hpp"
#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/spatial/al_HashSpace.hpp"
#include "al/ui/al_ControlGUI.hpp" 
#include "al/graphics/al_Font.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"
#include <iostream>
#include <fstream>
#include <vector>
using namespace al;
using namespace std;

const int birdsN = 240;
const int predatorsN = 10;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

string slurp(string fileName); 

HashSpace birdsSpace(6, birdsN);
HashSpace predatorsSpace(1, predatorsN);

struct SoundPlayer : SoundFile {
  int index{0};
  float operator()() {
    float value = data[index];
    index++;
    if (index < 1){
      return 0;
    }
    if (index > data.size())
      index = 0;
    return value;
  }
};

struct Birds : Pose {
  Vec3f heading;
  Vec3f center; 
  Vec3f velocity;
  Vec3f acceleration;
  unsigned flockCount{1};
};

struct Predators : Pose {
  Vec3f heading;
  Vec3f center; 
  Vec3f velocity;
  Vec3f acceleration;
  unsigned flockCount{1};
};

struct BirdsAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;

  BirdsAttribute (Vec3f p, Vec3f f, Vec3f u){
    position = p;
    forward = f;
    up = u;
  }
};

struct PredatorsAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;

  PredatorsAttribute (Vec3f p, Vec3f f, Vec3f u){
    position = p;
    forward = f;
    up = u;
  }
};

struct SharedState{
  BirdsAttribute birds[birdsN];
  PredatorsAttribute predators[predatorsN];
  float size;
  float ratio;
  float background;
};

class MyApp : public DistributedAppWithState<SharedState> {
  bool freeze = false;
  Font text;
  SoundPlayer fly;
  SoundPlayer eat;
  Parameter moveRate{"/moveRate", "", 1.0, "", 0.0, 2.0};
  Parameter turnRate{"/turnRate", "", 1.0, "", 0.0, 2.0};
  Parameter localRadius{"/localRadius", "", 0.4, "", 0.01, 0.9};
  ParameterInt k{"/k", "", 5, "", 1, 15};
  Parameter size{"/size", "", 1.0, "", 0.0, 2.0};
  Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  ShaderProgram birdsShader;
  ShaderProgram predatorsShader;
  Mesh birdsMesh;
  Mesh predatorsMesh;
  Mesh textMesh;

  vector<Birds> birds;
  vector<Predators> predators;

  float t = 0;
  int frameCount = 0;
  bool play_fly{false};

  void initBirds(){
    for (int _ = 0; _ < birdsN; _++) {
      Birds b;
      b.pos(rv());
      birdsSpace.move(_, b.pos() * birdsSpace.dim());
      b.faceToward(rv());
      birds.push_back(b);
      birdsMesh.vertex(b.pos());
      birdsMesh.normal(b.uf());
      const Vec3f& up(b.uu());
      birdsMesh.color(up.x, up.y, up.z);
    }
  }

  void initPredators(){
    for (int _ = 0; _ < predatorsN; _++) {
      Predators p;
      p.pos(rv());
      predatorsSpace.move(_, p.pos() * predatorsSpace.dim());
      p.faceToward(rv());
      predators.push_back(p);
      predatorsMesh.vertex(p.pos());
      predatorsMesh.normal(p.uf());
      const Vec3f& up(p.uu());
      predatorsMesh.color(up.x, up.y, up.z);
    }
  }

  void onCreate() override{
    cuttleboneDomain =
        CuttleboneStateSimulationDomain<SharedState>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }

    gui << moveRate << turnRate << localRadius << k << size << ratio;
    gui.init();
    navControl().useMouse(false);

    birdsShader.compile(slurp("../tetrahedron-vertex.glsl"),
                   slurp("../tetrahedron-fragment.glsl"),
                   slurp("../tetrahedron-geometry.glsl"));
    predatorsShader.compile(slurp("../tetrahedron-vertex1.glsl"),
                       slurp("../tetrahedron-fragment1.glsl"),
                       slurp("../tetrahedron-geometry1.glsl"));

    birdsMesh.primitive(Mesh::POINTS);
    predatorsMesh.primitive(Mesh::POINTS);

    initBirds();
    initPredators();

    fly.open("../fly.wav");
    eat.open("../eat.wav");

    nav().pos(0, 0, 10);
  }

  void onSound(AudioIOData& io) override {
    while (io()) {
      float f = play_fly ? fly() : eat();
      io.out(0) = f;
      io.out(1) = f;
    }
  }

  void setBirds(){
    for (unsigned i = 0; i < birdsN; i++) {
      birds[i].center = birds[i].pos();
      birds[i].heading = birds[i].uf();
      birds[i].flockCount = 1;
      birds[i].acceleration.zero();
    }
  }

  void setPredators(){
    for (unsigned i = 0; i < predatorsN; i++) {
      predators[i].center = predators[i].pos();
      predators[i].heading = predators[i].uf();
      predators[i].flockCount = 1;
      predators[i].acceleration.zero();
    }
  }

  float queryBirds(float sum){
    for (int i = 0; i < birdsN; i++) {
      HashSpace::Query query(k);
      int results = query(birdsSpace, birds[i].pos() * birdsSpace.dim(),
                          birdsSpace.maxRadius() * localRadius);
      for (int j = 0; j < results; j++) {
        int id = query[j]->id;
        birds[i].heading += birds[id].uf();
        birds[i].center += birds[id].pos();
        birds[i].flockCount++;
      }
      sum += birds[i].flockCount;
    }
    return sum;
  }

  void alignBirds(){
    for (unsigned i = 0; i < birdsN; i++) {
      if (birds[i].flockCount < 1) {
        printf("ERROR");
        fflush(stdout);
        exit(1);
      }

      if (birds[i].flockCount == 1) {
        birds[i].faceToward(Vec3f(0, 0, 0), 0.003 * turnRate);
        continue;
      }

      birds[i].center /= birds[i].flockCount;
      birds[i].heading /= birds[i].flockCount;

      float distance = (birds[i].pos() - birds[i].center).mag();

      birds[i].faceToward(birds[i].pos() + birds[i].heading, 0.003 * turnRate);
      birds[i].faceToward(birds[i].center, 0.003 * turnRate);
      birds[i].faceToward(birds[i].pos() - birds[i].center, 0.003 * turnRate);
    }
  }

  void accelerateBirds(){
    for (int i = 0; i < birdsN; i++) {
      birds[i].acceleration += birds[i].uf() * moveRate * 0.002;
      birds[i].acceleration += -birds[i].velocity * 0.1; 
    }
  }

  void acceleratePredators(){
    for (int i = 0; i < predatorsN; i++) {
      predators[i].acceleration += predators[i].uf() * moveRate * 0.006;
      predators[i].acceleration += -predators[i].velocity * 0.2; 
    }
  }

  void integrateBirds(){
    for (int i = 0; i < birdsN; i++) {
      birds[i].velocity += birds[i].acceleration;
      birds[i].pos() += birds[i].velocity;
    }
  }

  void integratePredators(){
    for (int i = 0; i < predatorsN; i++) {
      predators[i].velocity += predators[i].acceleration;
      predators[i].pos() += predators[i].velocity;
    }
  }

  void makespaceBirds(){
    for (unsigned i = 0; i < birdsN; i++) {
      Vec3d p = birds[i].pos();

      if (p.x > 1) p.x -= 1;
      if (p.y > 1) p.y -= 1;
      if (p.z > 1) p.z -= 1;
      if (p.x < 0) p.x += 1;
      if (p.y < 0) p.y += 1;
      if (p.z < 0) p.z += 1;

      birds[i].pos(p);
      birdsSpace.move(i, birds[i].pos() * birdsSpace.dim());
    }
  }

  void makespacePredators(){
    for (unsigned i = 0; i < predatorsN; i++) {
      Vec3d p = predators[i].pos();

      if (p.x > 1) p.x -= 1;
      if (p.y > 1) p.y -= 1;
      if (p.z > 1) p.z -= 1;
      if (p.x < 0) p.x += 1;
      if (p.y < 0) p.y += 1;
      if (p.z < 0) p.z += 1;

      predators[i].pos(p);
      predatorsSpace.move(i, predators[i].pos() * predatorsSpace.dim());
    }
  }

  void visualizeBirds(){
    vector<Vec3f>& v(birdsMesh.vertices());
    vector<Vec3f>& n(birdsMesh.normals());
    vector<Color>& c(birdsMesh.colors());
    for (unsigned i = 0; i < birdsN; i++) {
      v[i] = state().birds[i].position;
      n[i] = state().birds[i].forward;
      const Vec3d& up(state().birds[i].up;
      c[i].set(up.x, up.y, up.z);
    }
  }

  void visualizePredators(){
    vector<Vec3f>& v(predatorsMesh.vertices());
    vector<Vec3f>& n(predatorsMesh.normals());
    vector<Color>& c(predatorsMesh.colors());
    for (unsigned i = 0; i < predatorsN; i++) {
      v[i] = state().predators[i].position;
      n[i] = state().predators[i].forward;
      const Vec3d& up(state().predators[i].up;
      c[i].set(up.x, up.y, up.z);
    }
  }

  void dispelBirds(){
    for(unsigned i = 0; i < predatorsN; i++){
      for(unsigned j = 0; j < birdsN; j++){
        float distance = (predators[i].pos() - birds[j].pos()).mag();
        if(distance < 0.3){
          birds[j].faceToward(birds[j].pos() + birds[j].heading, 0.01 * turnRate);
          //birds[j].faceToward(birds[j].center, 0.01 * turnRate);
          //birds[j].faceToward(birds[j].pos() - birds[j].center, 0.01 * turnRate);
        }
      }
    }
  }

  void eatBirds(){
    for(unsigned i = 0; i < predatorsN; i++){
      for(unsigned j = 0; j < birdsN; j++){
        float distance = (predators[i].pos() - birds[j].pos()).mag();
        if(distance < 0.1){
          birds[j].pos() = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
          // erase birds[j]
          text.write(textMesh, "Predators are earing birds", 0.08f);
          play_fly = !play_fly;
        }
        else{
          text.write(textMesh, "Predators are searching birds", 0.08f);
        }
      }
    }
  }

  void onAnimate(double dt) override {
    text.load("../VeraMono.ttf", 28, 1024);
    t += dt;
    frameCount++;
    float sum = 0;

    if(t > 1){
      t -= 1;
      frameCount = 0;
    }

    if(freeze == false){
      if (cuttleboneDomain->isSender()) {
      setBirds();
      setPredators();

      sum = queryBirds(sum);
      alignBirds();

      accelerateBirds();
      acceleratePredators();

      integrateBirds();
      integratePredators();

      makespaceBirds();
      makespacePredators();

      dispelBirds();
      eatBirds();

      } 
      else { }

      visualizeBirds();
      visualizePredators();
    }
  }
  
  bool onKeyDown(const Keyboard& k) override{
    if(k.key() == ' '){
      freeze = !freeze;
    }
  }

  void onDraw(Graphics& g) override {
    g.clear(state().background, state().background, state().background);
    gl::depthTesting(true); 
    gl::blending(true);      
    gl::blendTrans();        

    g.shader(predatorsShader);
    g.shader().uniform("size", state().size * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(predatorsMesh);  // rendered with predatorsShader

    g.shader(birdsShader);
    g.shader().uniform("size", state().size * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(birdsMesh);  // rendered with birdsShader
    
    g.texture();
    text.tex.bind();
    g.draw(textMesh);
    text.tex.unbind();

    if (isPrimary()){
      gui.draw(g);
    }
  }
};

int main() {
  MyApp app;
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
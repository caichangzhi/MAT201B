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

const int birdsN = 150;
const int predatorsN = 3;
const int insectN = 100;
const int pestN = 20;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

string slurp(string fileName); 

HashSpace birdsSpace(6, birdsN);
HashSpace predatorsSpace(1, predatorsN);
HashSpace insectSpace(3, insectN);
HashSpace pestSpace(1, pestN);

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

struct Insect : Pose {
  Vec3f heading;
  Vec3f center; 
  Vec3f velocity;
  Vec3f acceleration;
  unsigned flockCount{1};
};

struct Pest : Pose {
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
};

struct PredatorsAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;
};

struct InsectAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;
};

struct PestAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;
};

struct SharedState{
  BirdsAttribute birds[birdsN];
  PredatorsAttribute predators[predatorsN];
  InsectAttribute insect[insectN];
  PestAttribute pest[pestN];
  float birdsSize;
  float predatorsSize;
  float insectSize;
  float ratio;
  float background;
};

class MyApp : public DistributedAppWithState<SharedState> {
  bool freeze = false;
  Font text;
  SoundPlayer fly;
  SoundPlayer eat;
  Parameter birdsMR{"/birdsMR", "", 0.4, "", 0.0, 1.5};
  Parameter predatorsMR{"/predatorsMR", "", 1.0, "", 0.0, 2.0};
  Parameter insectMR{"/insectMR", "", 0.2, "", 0.0, 1.0};
  Parameter birdsTR{"/birdsTR", "", 1.0, "", 0.0, 2.0};
  Parameter insectTR{"/insectTR", "", 0.6, "", 0.0, 1.5};
  Parameter birdsRadius{"/birdsRadius", "", 0.05, "", 0.01, 0.9};
  Parameter insectRadius{"/insectRadius", "", 0.02, "", 0.01, 0.5};
  ParameterInt k{"/k", "", 5, "", 1, 15};
  Parameter birdsSize{"/birdsSize", "", 1.0, "", 0.0, 2.0};
  Parameter insectSize{"/insectSize", "", 0.3, "", 0.0, 1.0};
  Parameter predatorsSize{"/predatorsSize", "", 1.5, "", 0.5, 2.0};
  Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  ShaderProgram birdsShader;
  ShaderProgram predatorsShader;
  ShaderProgram insectShader;
  ShaderProgram pestShader;
  Mesh birdsMesh;
  Mesh predatorsMesh;
  Mesh insectMesh;
  Mesh pestMesh;
  Mesh textMesh;

  vector<Birds> birds;
  vector<Predators> predators;
  vector<Insect> insect;
  vector<Pest> pest;

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

  void initInsect(){
    for (int _ = 0; _ < insectN; _++) {
      Insect i;
      i.pos(rv());
      insectSpace.move(_, i.pos() * insectSpace.dim());
      i.faceToward(rv());
      insect.push_back(i);
      insectMesh.vertex(i.pos());
      insectMesh.normal(i.uf());
      const Vec3f& up(i.uu());
      insectMesh.color(up.x, up.y, up.z);
    }
  }

  void initPest(){
    for (int _ = 0; _ < pestN; _++) {
      Pest p;
      p.pos(rv());
      pestSpace.move(_, p.pos() * pestSpace.dim());
      p.faceToward(rv());
      pest.push_back(p);
      pestMesh.vertex(p.pos());
      pestMesh.normal(p.uf());
      const Vec3f& up(p.uu());
      pestMesh.color(up.x, up.y, up.z);
    }
  }

  void onCreate() override{
    cuttleboneDomain =
        CuttleboneStateSimulationDomain<SharedState>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }

    gui << birdsMR << birdsTR << birdsRadius << birdsSize
    << predatorsMR << predatorsSize
    << insectMR << insectTR << insectRadius << insectSize
    << k << ratio;
    gui.init();
    navControl().useMouse(false);

    birdsShader.compile(slurp("../birds-vertex.glsl"),
                   slurp("../birds-fragment.glsl"),
                   slurp("../birds-geometry.glsl"));
    predatorsShader.compile(slurp("../predators-vertex.glsl"),
                       slurp("../predators-fragment.glsl"),
                       slurp("../predators-geometry.glsl"));
    insectShader.compile(slurp("../insect-vertex.glsl"),
                       slurp("../insect-fragment.glsl"),
                       slurp("../insect-geometry.glsl"));
    pestShader.compile(slurp("../pest-vertex.glsl"),
                       slurp("../pest-fragment.glsl"),
                       slurp("../pest-geometry.glsl"));

    birdsMesh.primitive(Mesh::POINTS);
    predatorsMesh.primitive(Mesh::POINTS);
    insectMesh.primitive(Mesh::POINTS);
    pestMesh.primitive(Mesh::POINTS);
    
    initBirds();
    initPredators();
    initInsect();
    initPest();

    fly.open("../fly.wav");
    eat.open("../eat.wav");

    nav().pos(0.5, 0.5, 10);
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

  void setInsect(){
    for (unsigned i = 0; i < insectN; i++) {
      insect[i].center = insect[i].pos();
      insect[i].heading = insect[i].uf();
      insect[i].flockCount = 1;
      insect[i].acceleration.zero();
    }
  }

  void setPest(){
    for (unsigned i = 0; i < pestN; i++) {
      pest[i].center = pest[i].pos();
      pest[i].heading = pest[i].uf();
      pest[i].flockCount = 1;
      pest[i].acceleration.zero();
    }
  }

  float queryBirds(float sum){
    for (int i = 0; i < birdsN; i++) {
      HashSpace::Query query(k);
      int results = query(birdsSpace, birds[i].pos() * birdsSpace.dim(),
                          birdsSpace.maxRadius() * birdsRadius);
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
        birds[i].faceToward(Vec3f(0, 0, 0), 0.003 * birdsTR);
        continue;
      }

      birds[i].center /= birds[i].flockCount;
      birds[i].heading /= birds[i].flockCount;

      float distance = (birds[i].pos() - birds[i].center).mag();

      birds[i].faceToward(birds[i].pos() + birds[i].heading, 0.003 * birdsTR);
      birds[i].faceToward(birds[i].center, 0.003 * birdsTR);
      birds[i].faceToward(birds[i].pos() - birds[i].center, 0.003 * birdsTR);
    }
  }

  void accelerateBirds(){
    for (int i = 0; i < birdsN; i++) {
      birds[i].acceleration += birds[i].uf() * birdsMR * 0.002;
      birds[i].acceleration += -birds[i].velocity * 0.1; 
    }
  }

  void acceleratePredators(){
    for (int i = 0; i < predatorsN; i++) {
      predators[i].acceleration += predators[i].uf() * predatorsMR * 0.002;
      predators[i].acceleration += -predators[i].velocity * 0.1; 
    }
  }

  void accelerateInsect(){
    for (int i = 0; i < insectN; i++) {
      insect[i].acceleration += insect[i].uf() * insectMR * 0.002;
      insect[i].acceleration += -insect[i].velocity * 0.1; 
    }
  }

  void acceleratePest(){
    for (int i = 0; i < pestN; i++) {
      pest[i].acceleration += pest[i].uf() * insectMR * 0.002;
      pest[i].acceleration += -pest[i].velocity * 0.1; 
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

  void integrateInsect(){
    for (int i = 0; i < insectN; i++) {
      insect[i].velocity += insect[i].acceleration;
      insect[i].pos() += insect[i].velocity;
    }
  }

  void integratePest(){
    for (int i = 0; i < pestN; i++) {
      pest[i].velocity += pest[i].acceleration;
      pest[i].pos() += pest[i].velocity;
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

  void makespaceInsect(){
    for (unsigned i = 0; i < insectN; i++) {
      Vec3d p = insect[i].pos();

      if (p.x > 1) p.x -= 1;
      if (p.y > 1) p.y -= 1;
      if (p.z > 1) p.z -= 1;
      if (p.x < 0) p.x += 1;
      if (p.y < 0) p.y += 1;
      if (p.z < 0) p.z += 1;

      insect[i].pos(p);
      insectSpace.move(i, insect[i].pos() * insectSpace.dim());
    }
  }

  void makespacePest(){
    for (unsigned i = 0; i < pestN; i++) {
      Vec3d p = pest[i].pos();

      if (p.x > 1) p.x -= 1;
      if (p.y > 1) p.y -= 1;
      if (p.z > 1) p.z -= 1;
      if (p.x < 0) p.x += 1;
      if (p.y < 0) p.y += 1;
      if (p.z < 0) p.z += 1;

      pest[i].pos(p);
      pestSpace.move(i, pest[i].pos() * pestSpace.dim());
    }
  }

  void birdsDistribute(){
    for (unsigned i = 0; i < birdsN; i++) { 
        BirdsAttribute b;
        b.position = birds[i].pos();
        b.forward = birds[i].uf();
        b.up = birds[i].uu();
        state().birds[i] = b;
      }
      state().birdsSize = birdsSize.get();
      state().ratio = ratio.get();
  }

  void predatorsDistribute(){
    for (unsigned i = 0; i < predatorsN; i++) { 
        PredatorsAttribute p;
        p.position = predators[i].pos();
        p.forward = predators[i].uf();
        p.up = predators[i].uu();
        state().predators[i] = p;
      }
      state().predatorsSize = predatorsSize.get();
      state().ratio = ratio.get();
  }

  void insectDistribute(){
    for (unsigned i = 0; i < insectN; i++) { 
        InsectAttribute ic;
        ic.position = insect[i].pos();
        ic.forward = insect[i].uf();
        ic.up = insect[i].uu();
        state().insect[i] = ic;
      }
      state().insectSize = insectSize.get();
      state().ratio = ratio.get();
  }

  void pestDistribute(){
    for (unsigned i = 0; i < pestN; i++) { 
        PestAttribute p;
        p.position = pest[i].pos();
        p.forward = pest[i].uf();
        p.up = pest[i].uu();
        state().pest[i] = p;
      }
      state().insectSize = insectSize.get();
      state().ratio = ratio.get();
  }

  void visualizeBirds(){
    vector<Vec3f>& v(birdsMesh.vertices());
    vector<Vec3f>& n(birdsMesh.normals());
    vector<Color>& c(birdsMesh.colors());
    for (unsigned i = 0; i < birdsN; i++) {
      v[i] = state().birds[i].position;
      n[i] = state().birds[i].forward;
      const Vec3d& up(state().birds[i].up);
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
      const Vec3d& up(state().predators[i].up);
      c[i].set(up.x, up.y, up.z);
    }
  }

  void visualizeInsect(){
    vector<Vec3f>& v(insectMesh.vertices());
    vector<Vec3f>& n(insectMesh.normals());
    vector<Color>& c(insectMesh.colors());
    for (unsigned i = 0; i < insectN; i++) {
      v[i] = state().insect[i].position;
      n[i] = state().insect[i].forward;
      const Vec3d& up(state().insect[i].up);
      c[i].set(up.x, up.y, up.z);
    }
  }

  void visualizePest(){
    vector<Vec3f>& v(pestMesh.vertices());
    vector<Vec3f>& n(pestMesh.normals());
    vector<Color>& c(pestMesh.colors());
    for (unsigned i = 0; i < pestN; i++) {
      v[i] = state().pest[i].position;
      n[i] = state().pest[i].forward;
      const Vec3d& up(state().pest[i].up);
      c[i].set(up.x, up.y, up.z);
    }
  }

  void preDispelBirds(){
    for(unsigned i = 0; i < predatorsN; i++){
      for(unsigned j = 0; j < birdsN; j++){
        float distance = (predators[i].pos() - birds[j].pos()).mag();
        if(distance < 0.25){
          birds[j].faceToward(birds[j].pos() - predators[i].heading * (0.5, 0.5, 0), 1.0 * birdsTR);
        }
      }
    }
  }

  void dispelInsect(){
    for(unsigned i = 0; i < birdsN; i++){
      for(unsigned j = 0; j < insectN; j++){
        float distance = (birds[i].pos() - insect[j].pos()).mag();
        if(distance < 0.20){
          insect[j].faceToward(insect[j].pos() - birds[i].heading * (0.5, 0.5, 0), 1.0 * birdsTR);
        }
      }
    }
  }

  void pestDispelBirds(){
    for(unsigned i = 0; i < pestN; i++){
      for(unsigned j = 0; j < birdsN; j++){
        float distance = (pest[i].pos() - birds[j].pos()).mag();
        if(distance < 0.15){
          birds[j].faceToward(birds[j].pos() - pest[i].heading * (0.5, 0.5, 0.5), 1.0 * birdsTR);
        }
      }
    }
  }

  void eatBirds(){
    for(unsigned i = 0; i < predatorsN; i++){
      for(unsigned j = 0; j < birdsN; j++){
        float distance = (predators[i].pos() - birds[j].pos()).mag();
        if(distance < birdsRadius){
          birds[j].pos() = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
          text.write(textMesh, "Predators are earing birds", 0.08f);
          play_fly = !play_fly;
        }
        else{
          text.write(textMesh, "Predators are searching birds", 0.08f);
        }
      }
    }
  }

  void eatInsect(){
    for(unsigned i = 0; i < birdsN; i++){
      for(unsigned j = 0; j < insectN; j++){
        float distance = (birds[i].pos() - insect[j].pos()).mag();
        if(distance < insectRadius){
          insect[j].pos() = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
          text.write(textMesh, "Birds are earing insects", 0.08f);
          play_fly = !play_fly;
        }
        else{
          text.write(textMesh, "Birds are searching insects", 0.08f);
        }
      }
    }
  }

  void eatPest(){
    for(unsigned i = 0; i < birdsN; i++){
      for(unsigned j = 0; j < pestN; j++){
        float distance = (birds[i].pos() - pest[j].pos()).mag();
        if(distance < insectRadius){
          birds[i].pos() = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
          text.write(textMesh, "Birds are infected by pest", 0.08f);
          play_fly = !play_fly;
        }
        else{
          text.write(textMesh, "Birds are searching pest", 0.08f);
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
      setInsect();
      setPest();

      sum = queryBirds(sum);
      alignBirds();

      accelerateBirds();
      acceleratePredators();
      accelerateInsect();
      acceleratePest();

      integrateBirds();
      integratePredators();
      integrateInsect();
      integratePest();

      makespaceBirds();
      makespacePredators();
      makespaceInsect();
      makespacePest();

      preDispelBirds();
      pestDispelBirds();
      dispelInsect();
      eatBirds();
      eatInsect();
      eatPest();

      birdsDistribute();
      predatorsDistribute();
      insectDistribute();
      pestDistribute();
      } 
      
      else { }

      visualizeBirds();
      visualizePredators();
      visualizeInsect();
      visualizePest();
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
    g.shader().uniform("size", state().predatorsSize * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(predatorsMesh);  // rendered with predatorsShader

    g.shader(birdsShader);
    g.shader().uniform("size", state().birdsSize * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(birdsMesh);  // rendered with birdsShader

    g.shader(insectShader);
    g.shader().uniform("size", state().insectSize * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(insectMesh);  

    g.shader(pestShader);
    g.shader().uniform("size", state().insectSize * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(pestMesh); 
    
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
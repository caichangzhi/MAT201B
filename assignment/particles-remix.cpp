// Assignment 3
// Changzhi Cai
// Modified kramerelwell's particle-p2.cpp
// Summary: Modify according to the actual size and color of each planet
//          in the solar system to make this system better reflect 
//          the real situation of the solar system

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp" 
using namespace al;

#include <fstream>
#include <vector>
using namespace std;

Vec3f rv(float scale) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

Vec3f sunDist(float scale, float distance) {
  return Vec3f(distance, 0, 0) * scale;
}

string slurp(string fileName);  

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 4.0, "", 0.0, 5.0};
  Parameter timeStep{"/timeStep", "", 0.1, "", 0.01, 3.0};
  ControlGUI gui;

  ShaderProgram pointShader;
  Mesh mesh;  

  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<Vec3f> gravitation;
  vector<float> mass;

  float initV = 1;
  const float gravCon = 1.190588106 * pow(10, -8);

  vector<float> planetM     {330000, 317.83, 95.162, 14.536, 17.147, 1, 0.815, 0.107, 0.0553, 0.0022};  // Earth Mass       
  vector<float> planetD     {0, 5.20, 9.58, 19.2, 30.1, 1, 0.722, 1.52, 0.387, 39.5};                   // AU Dist

  void reset() {
    mesh.reset();
    velocity.clear();
    acceleration.clear();

    for (int i = 0; i < planetM.size(); i++) {
      mesh.vertex(sunDist(1.0, planetD[i]));            
      mass.push_back(planetM[i]);                       
      mesh.texCoord(pow(planetM[i], 1.0 / 3.0), 0); // set the planets shape
      if(i == 0){
        mesh.color(RGB(0.98, 0.83, 0.25));          // set Sun's color
      }
      else if(i == 1){
        mesh.color(RGB(0.85, 0.80, 0.71));          // set Jupiter's color
      }
      else if(i == 2){
        mesh.color(RGB(0.80, 0.72, 0.72));          // set Saturn's color
      }
      else if(i == 3){
        mesh.color(RGB(0.83, 0.98, 0.98));          // set Uranus's color
      } 
      else if(i == 4){
        mesh.color(RGB(0.52, 0.68, 0.86));          // set Neptune's color
      }
      else if(i == 5){
        mesh.color(RGB(0.31, 0.30, 0.69));          // set Earth's color
      }
      else if(i == 6){
        mesh.color(RGB(0.54, 0.49, 0.51));          // set Venus's color
      }
      else if(i == 7){
        mesh.color(RGB(0.75, 0.27, 0.05));          // set Mars's color
      }
      else if(i == 8){
        mesh.color(RGB(0.86, 0.80, 0.79));          // set Mercury's color
      }
      else if(i == 9){
        mesh.color(RGB(0.99, 0.94, 0.83));          // set Pulto's color
      }

      Vec3f initV(0, sqrt((gravCon * planetM[0]) / planetD[i]), 0);
      velocity.push_back(initV);
      acceleration.push_back(0);
    }
  }
                           
  void onCreate() override {

    gui << pointSize << timeStep;
    gui.init();
    navControl().useMouse(false);

    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    mesh.primitive(Mesh::POINTS);
    reset();
    nav().pos(0, 0, 10);
  }


  bool pause = false;
  void onAnimate(double dt) override {
    if (pause) return;

    dt = timeStep;

    // Gravitation
    for (int i = 0; i < mass.size(); i++){
      for (int k = i + 1; k < planetM.size(); k++){
        Vec3f r = mesh.vertices()[i] - mesh.vertices()[k]; 
        //  Vec3f rNorm = r.normalize();
        double rDist = pow(r.mag(), 3.0);  
        if (rDist == 0) continue;

        Vec3f gravitation = gravCon * mass[i] * mass[k] * r / rDist;

        acceleration[i] -= gravitation; 
        acceleration[k] += gravitation; 
        }
    }

    vector<Vec3f>& position(mesh.vertices());
    for (int i = 0; i < velocity.size(); i++) {
      if (i == 0) {velocity[i] = Vec3f(0,0,0);}
      else {
        velocity[i] += acceleration[i] / mass[i] * dt;
        position[i] += velocity[i] * dt;
      }
    }

    for (auto& a : acceleration) a.zero();
  }


  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == ' ') {
      pause = !pause;
    }
    
    if (k.key() == 'r') {
      reset();
    }

    // when press '1' reverse the velocity
    if (k.key() == '1') {
      for (int i = 0; i < velocity.size(); i++) {
        velocity[i] = velocity[i] * (-1.0f);
      }
    }

    return true;
  }


  void onDraw(Graphics& g) override {
    g.clear(0.1);
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
    g.blending(true);
    g.blendModeTrans();
    g.depthTesting(true);
    g.draw(mesh);
    gui.draw(g);
  }
};

int main() { AlloApp().start(); }

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
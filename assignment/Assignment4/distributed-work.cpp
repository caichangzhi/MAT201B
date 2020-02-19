// MAT201B Assignment/4 distributed-work
// created by Changzhi Cai at Fed 18th 2020

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

// define the number of agents
const int N = 1000;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

string slurp(string fileName);  // forward declaration

// suggested structure for Agent
//
struct Agent : Pose {
  Vec3f heading, center;  // of the local flock! averages
  unsigned flockCount{1};
};

// define the array of agents
Agent agents[N];

// agent attributes for future use
struct AgentAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;
};

// define the SharedState structure
struct SharedState{
  AgentAttribute agents[N];
  float size;
  float ratio;
};

class MyApp : public DistributedAppWithState<SharedState> {
  // add more GUI here
  Parameter moveRate{"/moveRate", "", 1.0, "", 0.0, 2.0};
  Parameter turnRate{"/turnRate", "", 1.0, "", 0.0, 2.0};
  Parameter localRadius{"/localRadius", "", 0.4, "", 0.01, 0.9};
  Parameter size{"/size", "", 1.0, "", 0.0, 2.0};
  Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

  // You can keep a pointer to the cuttlebone domain
  // This can be useful to ask the domain if it is a sender or receiver
  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  ShaderProgram shader;
  Mesh mesh;

  // vector<Agent> agent;

  void onCreate() override {
    cuttleboneDomain =
        CuttleboneStateSimulationDomain<SharedState>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }

    // add more GUI here
    gui << moveRate << turnRate << localRadius << size << ratio;
    gui.init();
    navControl().useMouse(false);

    // compile shaders
    shader.compile(slurp("../tetrahedron-vertex.glsl"),
                   slurp("../tetrahedron-fragment.glsl"),
                   slurp("../tetrahedron-geometry.glsl"));

    mesh.primitive(Mesh::POINTS);

    for (int _ = 0; _ < 1000; _++) {
      Agent a;
      a.pos(rv());
      a.faceToward(rv());
      agents[_] = a;
      mesh.vertex(a.pos());
      mesh.normal(a.uf());
      const Vec3f& up(a.uu());
      mesh.color(up.x, up.y, up.z);
    }

    nav().pos(0, 0, 10);
  }

  void onAnimate(double dt) override {
    Vec3f steer(0, 0, 0);
    Vec3f diff(0, 0, 0);
    Vec3f center(0, 0, 0);
    int count = 0;
    int countx = 0;
    int county = 0;
    int countz = 0;
    if (cuttleboneDomain->isSender()) {

    // code is here
    //
    // separation: steer to avoid crowding local flockmates
    for (unsigned i = 0; i < N; i++) {
      for (unsigned j = 1 + i; j < N; j++) {
        float distance = (agents[j].pos() - agents[i].pos()).mag();
        if (distance < localRadius) {
          diff = agents[j].pos() - agents[i].pos();
          diff.normalize();
          steer = steer + diff;
        }
      }
      steer.normalize();
      agents[i].faceToward(-steer);
      steer = (0, 0, 0);
    }

    // alignment: steer towards the average heading of local flockmates
    // for (unsigned i = 0; i < N; i++){
    //   for (unsigned j = i + 1; j < N; j++) {
    //     float distance = (agent[j].pos() - agent[i].pos()).mag();
    //     if (distance < localRadius){
    //     diff = agent[j].uf() - agent[i].uf();
    //     diff.normalize();
    //     steer = steer + diff;
    //     }
    //   }
    //   steer.normalize();
    //   agent[i].faceToward(steer);
    //   steer = (0, 0, 0);
    // }

    // cohesion: steer to move towards the average position of local flockmates
    // for (unsigned i = 0; i < N; i++){
    //   for (unsigned j = i + 1; j < N; j++){
    //     float distance = (agent[j].pos() - agent[i].pos()).mag();
    //     if (distance < localRadius){
    //       count = count + 1;
    //       countx = countx + agent[j].pos().x;
    //       county = county + agent[j].pos().y;
    //       countz = countz + agent[j].pos().z;
    //     }
    //   }
    //   if(count != 0){
    //     center.x = countx / count;
    //     center.y = county / count;
    //     center.z = countz / count;
    //     center.normalize();
    //   }
    //   agent[i].faceToward(center);
    //   count = 0;
    //   countx = 0;
    //   county = 0;
    //   countz = 0;
    // }

    // move the agents along (KEEP THIS CODE)
    //
    for (unsigned i = 0; i < N; i++) {
      agents[i].pos() += agents[i].uf() * moveRate * 0.002;
    }

    // respawn agents if they go too far (MAYBE KEEP)
    //
    for (unsigned i = 0; i < N; i++) {
      if (agents[i].pos().mag() > 1.1) {
        agents[i].pos(rv());
        agents[i].faceToward(rv());
      }
    }

    // change it to Distributed array
    for (unsigned i = 0; i < N; i++) { 
        AgentAttribute a;
        a.position = agents[i].pos();
        a.forward = agents[i].uf();
        a.up = agents[i].uu();
        state().agents[i] = a;
      }
      state().size = size.get();
      state().ratio = ratio.get();
    }
    else{ }

    // visualize the agents
    //
    vector<Vec3f>& v(mesh.vertices());
    vector<Vec3f>& n(mesh.normals());
    vector<Color>& c(mesh.colors());
    for (unsigned i = 0; i < N; i++) {
      v[i] = state().agents[i].position;
      n[i] = state().agents[i].forward;
      const Vec3d& up(state().agents[i].up);
      c[i].set(up.x, up.y, up.z);
    }
  }

  void onDraw(Graphics& g) override {
    g.clear(0.1, 0.1, 0.1);
    gl::depthTest(true); // or g.depthTesting(true);
    gl::blending(true); // or g.blending(true);
    gl::blendTrans(); // or g.blendModeTrans();
    g.shader(shader);
    g.shader().uniform("size", state().size * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(mesh);

    if (isPrimary()) {
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
#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

const int predatorsN = 100;

Predators predators[predatorsN];
Mesh predatorsMesh;
ShaderProgram predatorsShader;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

struct Predators : Pose {
  Vec3f heading;
  Vec3f center; 
  unsigned flockCount{1};

  Predators(){reset();}

  void reset(){
      pos(rv());
      faceToward(rv());
  }
};

struct PredatorsAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;

  PredatorsAttribute(){}
};

void initPredators(){
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
}
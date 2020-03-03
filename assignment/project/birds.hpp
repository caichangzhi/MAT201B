#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"

using namespace al;

const int birdsN = 100;

Birds birds[birdsN];
Mesh birdsMesh;
ShaderProgram birdsShader;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

struct Birds : Pose {
  Vec3f heading;
  Vec3f center; 
  unsigned flockCount{1};

  Birds(){reset();}

  void reset(){
      pos(rv());
      faceToward(rv());
  }
};

struct BirdsAttribute{
  Vec3f position;
  Vec3f forward;
  Vec3f up;

  BirdsAttribute(){}
};

void initBirds(){
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
}
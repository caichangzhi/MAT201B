#include "al/app/al_App.hpp"
#include "predators.hpp"
#include "birds.hpp"

using namespace al;

 void separation() {
    Vec3f steer(0, 0, 0);
    Vec3f diff(0, 0, 0);

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
  }
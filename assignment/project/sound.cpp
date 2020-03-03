#include "al/app/al_App.hpp"
#include "al/sound/al_SoundFile.hpp"
using namespace al;

struct SoundPlayer : SoundFile {
  int index{0};
  float operator()() {
    float value = data[index];
    index++;
    if (index > data.size())  //
      index = 0;
    return value;
  }
};

struct Appp : App {
  SoundPlayer a, e;

  bool play_a{false};

  void onCreate() override {
    a.open("../a.wav");
    e.open("../e.wav");
  }

  void onSound(AudioIOData& io) override {
    while (io()) {
      float f = play_a ? a() : e();
      io.out(0) = f;
      io.out(1) = f;
    }
  }

  bool onKeyDown(const Keyboard& key) override {
    play_a = !play_a;
    return true;
  }
};

int main() {
  Appp app;
  app.start();
}

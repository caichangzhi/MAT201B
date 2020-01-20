// MAT201B assignment/1
// created by Changzhi Cai at Jan 19th 2020

#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"  // al::Image
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

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

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 0.21, "", 0.0, 1.0};
  ControlGUI gui;
  ShaderProgram pointShader;
  Mesh mesh;
  vector<Vec3f> original;
  vector<Vec3f> rgbSpace;
  vector<Vec3f> hsvSpace;
  vector<Vec3f> restore;

  void onCreate() override {
    
    gui << pointSize;
    gui.init();
    navControl().useMouse(false);

    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    mesh.primitive(Mesh::POINTS);

    const char* filename = "../color_spaces.jpg";
    auto imageData = Image(filename);
    if (imageData.array().size() == 0) {
      std::cout << "failed to load image" << std::endl;
      exit(1);
    }

    Image::RGBAPix pixel;
    const int W = imageData.width();
    const int H = imageData.height();
    for (int c = 0; c < W; c++)
      for (int r = 0; r < H; r++) {
        imageData.read(pixel, c, r);

        // position returns points position of each pixel in original image
        Vec3f position((c - W / 2) * 0.005, (r - H / 2) * 0.005, 0);
        original.push_back(position);

        // position2 puts each point in a RGB color space
        Vec3f position2;
        position2.x = (pixel.r - 255.0 / 2) * 0.005 - 0.5;
        position2.y = (pixel.g - 255.0 / 2) * 0.005 - 0.5;
        position2.z = (pixel.b - 255.0 / 2) * 0.005 - 0.5;
        rgbSpace.push_back(position2);

        // position3 puts each point in a HSV color space
        Vec3f position3;
        HSV color(RGB(pixel.r, pixel.g, pixel.b));
        position3.x = color.s * sin(M_2PI * color.h);
        position3.y = color.s * cos(M_2PI * color.h);
        position3.z = color.v / 255.0;
        hsvSpace.push_back(position3);

        // position4 inverses the result of position, which looks like the original image
        Vec3f position4;
        position4.x = -position.x;
        position4.y = -position.y;
        position4.z = -position.z;
        restore.push_back(position4);

        mesh.vertex(position);
        mesh.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);
      }

    nav().pos(0, 0, 5);
  }

  bool freeze = false;
  double time = 0;
  void onAnimate(double dt) override {
    time += dt;
    if (freeze) return;

    // c++11 "lambda" function
    auto rv = []() {
      return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    };

    // range-for to do a random walk for each vertex
    for (auto& v : mesh.vertices()) {
      v += rv() * 0.01;
    }
  }

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == ' ') {
      freeze = !freeze;
    }

    // key '1' returns points position of each pixel in original image
    if (k.key() == '1') {
      for (int i = 0; i < mesh.vertices().size(); i++) {
        mesh.vertices()[i] = original[i];
      }
    }

    // key '2' puts each point in a RGB color space
    if (k.key() == '2'){
      for (int i = 0; i < mesh.vertices().size(); i++) {
        mesh.vertices()[i] = rgbSpace[i];
      }
    }

    // key '3' puts each point in a RGB color space
    if (k.key() == '3'){
      for (int i = 0; i < mesh.vertices().size(); i++) {
        mesh.vertices()[i] = hsvSpace[i];
      }
    }

    // key '4' inverses the result of position, which looks like the original image
    if (k.key() == '4'){
      for (int i = 0; i < mesh.vertices().size(); i++) {
        mesh.vertices()[i] = restore[i];
      }
    }
    return true;
  }

  void onDraw(Graphics& g) override {
    g.clear(0.01);
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
    g.shader().uniform("time", (float)time);
    g.depthTesting(true);
    g.draw(mesh);
    gui.draw(g);
  }
};

int main() { AlloApp().start(); }
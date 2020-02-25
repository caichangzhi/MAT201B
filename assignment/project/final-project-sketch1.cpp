#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp" 
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp" 

#include <fstream>
#include <vector>

using namespace al;
using namespace std;

float pix(float value, float low, float high, float Low, float High) {
    return Low + (High - Low) * ((value - low) / (high - low));
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

struct AlloApp : App {
    Parameter pointSize{"/pointSize", "", 0.21, "", 0.0, 1.0};
    vector<Vec3f> position[16];
    ControlGUI gui;
    ShaderProgram pointShader;
    Mesh mesh1;
    vector<Vec3f> original1;
    vector<Vec3f> rgbSpace1;
    vector<Vec3f> hsvSpace1;

 void onCreate() override { 
    gui << pointSize;
    gui.init();
    navControl().useMouse(false);

    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    mesh1.primitive(Mesh::POINTS);

    const char* filename1 = "../img/img1.jpg";
        auto imageData1 = Image(filename1);
        if (imageData1.array().size() == 0) {
          std::cout << "failed to load image" << std::endl;
          exit(1);
        }

    Image::RGBAPix pixel;

        const int W1 = imageData1.width();
        const int H1 = imageData1.height();
        for(int r = 0; r < H1; r++){
            for(int c = 0; c < W1; c++){
                imageData1.read(pixel, c, r);
                Vec3f o1;
                o1.x = pix(c, 0, W1, -1, 1);
                o1.y = pix(r, 0, H1, -1, 1);
                o1.z = 0;
                position[0].push_back(o1);

                Vec3f h1;
                HSV color(RGB(pixel.r, pixel.g, pixel.b));
                h1.x = color.s * sin(M_2PI * color.h);
                h1.y = color.s * cos(M_2PI * color.h);
                h1.z = color.v / 255;
                position[1].push_back(h1);

                mesh1.vertex(o1);
                float gray = HSV(RGB(pixel.r / 255, pixel.g / 255, pixel.b / 255)).v;
                gray = pix(gray, 0, 1, 0, 1.2);
                mesh1.color(Color(gray));
            }
        }
   }

    bool freeze = false;
    double time = 0;
    float timestep = 2;
    void onAnimate(double dt) override{
        auto& m1 = mesh1.vertices();
        time += dt / timestep;

        for(int i = 0; i < m1.size(); i++){
            if(time < 10){
                timestep = 2;
                m1[i] = position[0][i] * (10 - time) + position[1][i] * time;
            }
            else{
                m1[i] = position[1][i] * (20 - time) + position[0][i] * (time - 10);
            }
        }
    }

    void onDraw(Graphics& g) override {
    g.clear(0.01);
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
    g.shader().uniform("time", (float)time);
    g.depthTesting(true);
    g.draw(mesh1);
    gui.draw(g);
  }
};

int main() { AlloApp().start();}
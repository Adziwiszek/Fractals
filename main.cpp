#include <iostream>
#include <SFML/Graphics.hpp>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <thread>
#include <format>
#include <cmath>

#define DEB(text) std::cout << "D! " << text << std::endl;

constexpr int WIDTH{1600};
constexpr int HEIGHT{920};
constexpr int PIXEL_SIZE{1};

int gridWidth{WIDTH / PIXEL_SIZE};
int gridHeight{HEIGHT / PIXEL_SIZE};

typedef sf::Vector2f Vec2;
typedef sf::Vector2i IVec2;
typedef sf::Vector3f Vec3;

void setPixelColor(const int x, const int y, sf::Color color); 
Vec3 getGradientColor(const int x, const int max);

Vec2 loadCFromFile(std::string filename) {
  std::ifstream inp(filename);
  float x, y;
  inp >> x >> y;
  inp.close();
  return {x, y};
}

// returns the mod squared
float mod2(Vec2 z) {
  return z.x * z.x + z.y * z.y;
}

class Fractal {
  public:
    sf::VertexArray grid;
    Fractal(const int W, const int H);
    void render(IVec2 renderSize, Vec2 constant);
    void renderT(IVec2 renderSize, int startY, int endY, Vec2 constant);
    void randomRender(IVec2 renderSize);
    void mtRender(unsigned long thdsN, IVec2 renderSize, Vec2 constant);

  private:
    sf::Vector2u size;

    Vec2 computeNext(Vec2 current, Vec2 constant);
    int computeIterations(Vec2 z0, Vec2 constant, int maxIterations);
    float computeIterationsSmooth(Vec2 z0, Vec2 constant, int maxIterations);
    void setPixelColor(const int x, const int y, Vec3 color);
};


int main() {
  srand(time(NULL));
  sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Fractals");
  window.setFramerateLimit(60);
  DEB(gridHeight);
  DEB(gridWidth);
  auto fractal = Fractal(gridWidth, gridHeight);
  Vec2 c = loadCFromFile("const.txt"); 
  DEB("starting...");
  fractal.mtRender(50, {gridWidth, gridHeight}, c);
  //fractal.render({gridWidth, gridHeight}, c);
  DEB("done!");
  //fractal.randomRender({gridWidth, gridHeight});
  bool up{true};

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
      if(event.type == sf::Event::KeyPressed && 
          event.key.code == sf::Keyboard::Q)
        window.close();
    }
    /*if(!up)
      c += Vec2(-0.01, 0.01);  
      else 
      c += Vec2(0.01, -0.01);  
      fractal.render({gridWidth, gridHeight}, c);
      if(c.x >= 1) up = false;
      if(c.x <= -1) up = true;
      DEB(c.x);*/
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    window.clear(sf::Color::Black);
    window.draw(fractal.grid); 
    window.display();
  }
  return 0;
}

Fractal::Fractal(const int W, const int H)
  : grid{sf::PrimitiveType::Points, size_t(W * H)},
  size{unsigned(W), unsigned(H)} {
    uint32_t idx{0};
    for(uint32_t x{0}; x < W; x++) {
      for(uint32_t y{0}; y < H; y++) {
        grid[x + W * y].position = 
          sf::Vector2f{static_cast<float>(x), static_cast<float>(y)};
      }
    }
  }

Vec2 Fractal::computeNext(Vec2 current, Vec2 constant) {
  float zr = current.x * current.x  - current.y * current.y;
  float zi = 2.0 * current.x * current.y;

  return Vec2{zr, zi} + constant;
}

int Fractal::computeIterations(Vec2 z0, Vec2 constant, int maxIterations) {
  int iterations{0};
  Vec2 zn{z0};
  while(mod2(zn) < 4 && iterations++ <= maxIterations) {
    zn = computeNext(zn, constant);
  }
  return iterations;
}

float Fractal::computeIterationsSmooth(Vec2 z0, Vec2 constant, int maxIterations) {
  int iterations{0};
  Vec2 zn{z0};
  while(mod2(zn) < 4 && iterations++ <= maxIterations) {
    zn = computeNext(zn, constant);
  }

  const float mod = sqrt(mod2(zn));
  const float smoothIteration = float(iterations) - log2(fmax(1.0f, log2(mod)));
  return iterations;
}


void Fractal::mtRender(unsigned long thdsN, IVec2 renderSize, Vec2 constant) {
  auto thread_render = 
    [this] (IVec2 renderSize, int startY, int endY, Vec2 constant) {
      this->renderT(renderSize, startY, endY, constant);
    };
  std::vector<std::thread> threads;
  int start = 0;
  int step = 860 / thdsN;
  int end = 860;
  int currentStart = 0;
  int currentEnd = currentStart + step;

  for(unsigned long i = 0; i < thdsN-1; i++) {
    threads.push_back( 
      std::thread(thread_render, renderSize, currentStart, currentEnd, constant));
    currentStart += step;
    currentEnd += step;
  }
  threads.push_back( 
    std::thread(thread_render, renderSize, currentStart, currentEnd + (860 % step), constant)); 

  for(unsigned long i = 0; i < thdsN; i++) {
    threads[i].join();
  }
}

void Fractal::renderT(IVec2 renderSize, int startY, int endY, Vec2 constant){
  const float scale = 1.0f / (float(renderSize.y) / 2.0);
  const int maxIter{500};
  auto getRand = [](){ return float(rand()%99) / 100.0f; };

  for(int y{startY}; y < endY; y++) {
    for(int x{0}; x < renderSize.x; x++) {
      const int samples{16};
      Vec3 color;
      for(int i{samples}; i--;) {
        // compute pixel coordinates
        float px {(float(x - renderSize.x / 2.0) + getRand()) * scale};
        float py {(float(y - renderSize.y / 2.0) + getRand()) * scale};
        // set color based on iterations
        const float iterations = computeIterationsSmooth({px, py}, constant, maxIter);
        color += getGradientColor(iterations, maxIter); 
      }
      setPixelColor(x, y, color / float(samples)); 
    }
  }
}

void Fractal::render(IVec2 renderSize, Vec2 constant) {
  const float scale = 1.0f / (float(renderSize.y) / 2.0);
  const int maxIter{500};
  auto getRand = [](){ return float(rand()%99) / 100.0f; };
  for(int y{0}; y < renderSize.y; y++) {
    for(int x{0}; x < renderSize.x; x++) {
      const int samples{16};
      Vec3 color;
      for(int i{samples}; i--;) {
        // compute pixel coordinates
        float px {(float(x - renderSize.x / 2.0) + getRand()) * scale};
        float py {(float(y - renderSize.y / 2.0) + getRand()) * scale};
        // set color based on iterations
        const float iterations = computeIterationsSmooth({px, py}, constant, maxIter);
        color += getGradientColor(iterations, maxIter); 
      }
      setPixelColor(x, y, color / float(samples)); 
    }
  }
}

void Fractal::randomRender(IVec2 renderSize) {
  const float scale{1.0f / float(static_cast<float>(renderSize.y) / 2.0)};

  const int maxIter{50};
  for(int y{0}; y < renderSize.y; y++) {
    for(int x{0}; x < renderSize.x; x++) {

      // compute pixel coordinates
      float px {float(x - renderSize.x / 2.0) * scale};
      float py {float(y - renderSize.y / 2.0) * scale};
      // set color based on iterations
      setPixelColor(x, y, getGradientColor(rand() % 49 + 1, maxIter)); 
    }
  }
}

void Fractal::setPixelColor(const int x, const int y, Vec3 color) {
  int idx {x + y * int(size.x)};
  grid[idx].color = sf::Color(color.x, color.y, color.z);
}

Vec3 getGradientColor(const int x, const int max) {
  float scale {float(x) / float(max)};
  return {255 * scale, 255 * scale, 255 * scale};
}

#include <iostream>
#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>
#include <unistd.h>

#define DEB(text) std::cout << "D! " << text << std::endl;

const int WIDTH{800};
const int HEIGHT{600};
const int PIXEL_SIZE{1};

int gridWidth{WIDTH / PIXEL_SIZE};
int gridHeight{HEIGHT / PIXEL_SIZE};

typedef sf::Vector2f Vec2;
typedef sf::Vector2i IVec2;

void setPixelColor(const int x, const int y, sf::Color color); 
sf::Color getGradientColor(const int x, const int max);

// returns the mod squared
float mod2(Vec2 z) {
    return z.x * z.x + z.y * z.y;
}

class Fractal {
public:
    sf::VertexArray grid;
    Fractal(const int W, const int H);
    void render(IVec2 renderSize, Vec2 constant);

private:
    sf::Vector2u size;

    Vec2 computeNext(Vec2 current, Vec2 constant);
    int computeIterations(Vec2 z0, Vec2 constant, int maxIterations);
    void setPixelColor(const int x, const int y, sf::Color color);
};


int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Fractals");
    window.setFramerateLimit(60);
    DEB(gridHeight);
    DEB(gridWidth);
    auto fractal = Fractal(gridWidth, gridHeight);
    fractal.render({gridWidth, gridHeight}, {-0.78, 0.1});
    Vec2 c{1.0, -1.0};
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
        if(!up)
            c += Vec2(-0.01, 0.01);  
        else 
            c += Vec2(0.01, -0.01);  
        fractal.render({gridWidth, gridHeight}, c);
        if(c.x >= 0.5) up = false;
        if(c.x <= -0.5) up = true;
        DEB(c.x);
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
            grid[idx++].position = 
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


void Fractal::render(IVec2 renderSize, Vec2 constant) {
    const float scale{1.0f / float(static_cast<float>(renderSize.y) / 2.0)};
    const int maxIter{50};
    for(int y{0}; y < renderSize.y; y++) {
        for(int x{0}; x < renderSize.x; x++) {
            // compute pixel coordinates
            float px {float(x - renderSize.x / 2.0) * scale};
            float py {float(y - renderSize.y / 2.0) * scale};
            const float iterations = computeIterations({px, py}, constant, maxIter);
            setPixelColor(x, y, getGradientColor(iterations, maxIter)); 
        }
    }
}

void Fractal::setPixelColor(const int x, const int y, sf::Color color) {
    int idx {x + y * int(size.x)};
    grid[idx].color = color;
}

sf::Color getGradientColor(const int x, const int max) {
    float scale {float(x) / float(max)};
    return sf::Color(255 * scale, 255 * scale, 255 * scale);
}

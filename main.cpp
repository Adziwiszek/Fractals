#include <iostream>
#include <SFML/Graphics.hpp>

#define DEB(text) std::cout << "D! " << text << std::endl;

const int WIDTH{800};
const int HEIGHT{600};
const int PIXEL_SIZE{20};

int gridWidth{WIDTH / PIXEL_SIZE};
int gridHeight{HEIGHT / PIXEL_SIZE};

typedef sf::Vector2f Vec2;
typedef sf::Vector2i IVec2;

sf::VertexArray makeGrid(const int W, const int H);
void setPixelColor(const int x, const int y, sf::Color color); 
sf::Color getGradientColor(const int x, const int max);

// returns the mod squared
float mod2(Vec2 z) {
    return z.x * z.x + z.y * z.y;
}

class Fractal {
public:
    Fractal();
    void render(IVec2 renderSize, Vec2 constant);
private:
    sf::VertexArray grid = makeGrid(gridWidth, gridHeight);
    
    Vec2 computeNext(Vec2 current, Vec2 constant);
    int computeIterations(Vec2 z0, Vec2 constant, int maxIterations);
};

sf::VertexArray grid = makeGrid(gridWidth, gridHeight);


void render(IVec2 renderSize, Vec2 constant) {
    const float scale{1.0f / float(static_cast<float>(renderSize.y) / 2.0)};
    const int maxIter{50};
    DEB("start rendering..."); 
    for(int y{0}; y < renderSize.y; y++) {
        for(int x{0}; x < renderSize.x; x++) {
            // compute pixel coordinates
            float px {float(x - renderSize.x) * scale};
            float py {float(y - renderSize.y) * scale};
            int gridX {x / PIXEL_SIZE};
            int gridY {y / PIXEL_SIZE};
            // compute color
            if(gridX < 0 || gridX >= gridHeight || gridY < 0 || gridY >= gridHeight) {
                DEB(gridX);
                continue;
            }
            const float iterations = computeIterations({px, py}, constant, maxIter);
            DEB(iterations);
            setPixelColor(px, py, getGradientColor(iterations, maxIter)); 
        }
    }
    DEB("finished!");
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Fractals");
    window.setFramerateLimit(60);


    //sf::VertexArray grid = makeGrid(gridWidth, gridHeight);
    render({800, 600}, {1.0, 2.0});

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::KeyPressed && 
                    event.key.code == sf::Keyboard::Q)
                window.close();
        }

        window.clear(sf::Color::Black);
        
        window.draw(grid);

        window.display();
    }
    return 0;
}

Fractal::Fractal() {}


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

sf::VertexArray makeGrid(const int W, const int H) {
    sf::VertexArray grid(sf::Quads, W * H * 4);   
    for (int x = 0; x < W; ++x) {
        for (int y = 0; y < H; ++y) {
            int index{(x + y * W) * 4};
        
            // Define the quad for the current pixel
            grid[index + 0].position = sf::Vector2f(x * PIXEL_SIZE, y * PIXEL_SIZE);
            grid[index + 1].position = sf::Vector2f((x + 1) * PIXEL_SIZE, y * PIXEL_SIZE);
            grid[index + 2].position = sf::Vector2f((x + 1) * PIXEL_SIZE, (y + 1) * PIXEL_SIZE);
            grid[index + 3].position = sf::Vector2f(x * PIXEL_SIZE, (y + 1) * PIXEL_SIZE);

            // Set an initial color for each pixel
            grid[index + 0].color = sf::Color::White;
            grid[index + 1].color = sf::Color::White;
            grid[index + 2].color = sf::Color::White;
            grid[index + 3].color = sf::Color::White;
        }
    }
    return grid;
}

void setPixelColor(const int x, const int y, sf::Color color) {
    int index {(x + y * gridWidth) * 4};
    grid[index + 0].color = color;
    grid[index + 1].color = color;
    grid[index + 2].color = color;
    grid[index + 3].color = color;
}

sf::Color getGradientColor(const int x, const int max) {
    float scale {float(x) / float(max)};
    return sf::Color(255 * scale, 255 * scale, 255 * scale);
}

#ifndef APP_HH
#define APP_HH

#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>
#include "State.hh"

#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 640

// uses Eigen::Vector2d, Eigen::Vector2i
class App {
public:
    App();
    ~App();
    bool run(std::string filename);
private:
    State *state;
    SDL_Surface *buffer = nullptr;
    SDL_Texture *buffTex = nullptr;
    SDL_Texture *sky = nullptr;
    Mix_Music *music = nullptr;
    std::vector<SDL_Surface *> textures = std::vector<SDL_Surface *> (9, nullptr);
    uint32_t theTexture[64][64];
    void makeGlyphs(std::string fontname);               // helper to initialize
    void initialize(std::string filename);
    void drawTexture(
        int x, int side, int lineheight, double perpWallDist,
        int drawstart, int drawend,
        Eigen::Vector2d& ray, Eigen::Vector2i& mapPos);  // helper to drawLine
    void drawLine(int x);                                // helper to render3d
    void render3d();
    void render2d();
    void displayFPS(double fps);
    void getEvents();
    // Vector2d rotate2d(Vector2d vector, double rotSpeed)  // helper to updateData
    void updateData(double frameTime);
};

#endif  // APP_HH

#ifndef APP_HH
#define APP_HH


#include "State.hh"
#include "Matrix.hh"               // Vector2d Vector2i

#include <SDL2/SDL.h>
// TBD: include in App.cc along with SDL_ttf.h?
//#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>

#include <cstdint>                 // uint32_t

#include <string>
#include <vector>


// size in pixels (window not created with SDL_WINDOW_ALLOW_HIGHDPI, see:
//   - https://wiki.libsdl.org/SDL_GetWindowSize )
static constexpr uint32_t WINDOW_WIDTH  { 1080 };
static constexpr uint32_t WINDOW_HEIGHT { 640 };

static constexpr char FONT_FILE_PATH[]  { "AmAzing/fonts/Courier New.ttf" };
static constexpr char MUSIC_FILE_PATH[] { "AmAzing/audio/Game_of_Thrones.wav" };


class App {
public:
    App();
    ~App();

    void run(std::string filename);

private:
    State *state         { nullptr };

    SDL_Surface *buffer         { nullptr };
    SDL_Texture *buffer_texture { nullptr };
    SDL_Texture *sky            { nullptr };
    Mix_Music   *music          { nullptr };
    std::vector<SDL_Surface *> textures (9, nullptr);
    // uint32_t theTexture[64][64];  // TBD: unused in original source

    // void makeGlyphs(std::string fontname);               // helper to initialize
    void initialize(std::string filename);

    // void drawTexture(
    //    int x, int side, int lineheight, double perpWallDist,
    //    int drawstart, int drawend,
    //    Vector2d& ray, Vector2i& mapPos);                // helper to drawLine
    // void drawLine(int x);                                // helper to render3d
    void render3d();

    void displayFPS(double fps);
    void render2d();

    void getEvents();

    Vector2d rotate2d(Vector2d vector, double rotSpeed)  // helper to updateData
    void updateData(double frameTime);
};


#endif  // APP_HH

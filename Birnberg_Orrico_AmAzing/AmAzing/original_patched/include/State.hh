#ifndef STATE_HH
#define STATE_HH

#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <Dense>  // Eigen/Dense

#include <vector>
#include <unordered_map>

#include "KeyHandler.hh"
#include "Layout.hh"

// uses Eigen::Vector2d

class State {
private:
    // TBD: why a static pointer to its own class? to ensure that only one can
    //   be referenced no matter how many times the constructor is called?
    static State *instance;
public:
    ~State();
    bool done = false;
    bool showFPS = false;
    bool showMap = false;
    Eigen::Vector2d pos;
    Eigen::Vector2d dir;
    Eigen::Vector2d viewPlane;
    SDL_Renderer *renderer = nullptr;
    SDL_Window *window = nullptr;
    TTF_Font *font = nullptr;
    SDL_Texture *fpsTex = nullptr;
    // TBD: where is u_char defined?
    // TBD: why are we mapping textures to chars? and why unordered?
    std::unordered_map<u_char, SDL_Texture *> fontCache;
    KeyHandler keyHandler;
    Layout *layout = nullptr;
    static State *getInstance();
};

#endif  // STATE_HH

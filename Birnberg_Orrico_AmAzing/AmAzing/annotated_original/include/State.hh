#ifndef STATE_HH
#define STATE_HH


#include "KeyHandler.hh"
#include "Layout.hh"
#include "Matrix.hh"           // Vector2d

#include <SDL2/SDL.h>
// TBD: include in State.cc?
// #include <SDL2_ttf/SDL_ttf.h>

#include <cstdint>             // uint8_t

#include <vector>
#include <unordered_map>


class State {
private:
    // State in original source exhibits singleton class design pattern, to
    //   ensure only one instance of State can exist at once.
    // TBD: Is singleton pattern necessary?
    static State *instance;
public:
    // game state flags
    bool done     { false };
    bool show_fps { false };
    bool show_map { false };

    // raycasting engine state
    Vector2d pos;
    Vector2d dir;
    Vector2d view_plane;

    // SDL2 A/V
    SDL_Window   *window      { nullptr };
    SDL_Renderer *renderer    { nullptr };
    // TTF_Font     *font        { nullptr };  // TBD: unused?
    SDL_Texture  *fps_texture { nullptr };
    // original uses u_char, defined in sys/types.h, which is likely included
    //   by X11 headers included by SDL2; instead using uint8_t for consistency
    std::unordered_map<uint8_t, SDL_Texture *> font_cache;

    // user input
    KeyHandler key_handler;

    // map
    Layout *layout { nullptr };

    // TBD: rule of 5?
    ~State();

    static State *getInstance();
};

#endif  // STATE_HH

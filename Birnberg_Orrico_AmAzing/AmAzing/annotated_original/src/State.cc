#include "State.hh"  // SDL_* (via SDL2/SDL.h)


// Singleton instantiation pattern
State *State::instance { new State };

// Singleton getter
State *State::getInstance() {
    return instance;
}

State::~State() {
    if (window != nullptr)
        SDL_DestroyWindow(window);
    if (renderer != nullptr)
        SDL_DestroyRenderer(renderer);
    if (fps_texture != nullptr)
        SDL_DestroyTexture(fps_texture);
    // C++17 would allow structured binding: `for (auto& [key, value] : font_cache)
    for (const auto &kv : font_cache)
        SDL_DestroyTexture(kv.second);
    if (layout != nullptr)
        delete layout;
}

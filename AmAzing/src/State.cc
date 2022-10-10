#include "../include/State.hh"

State *State::instance = new State;

State *State::getInstance() {
    return instance;
}

State::~State() {
    if (window)
        SDL_DestroyWindow(window);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    // TBD: why no destruction of font?
    if (fpsTex)
        SDL_DestroyTexture(fpsTex);
    for (auto el : fontCache) {
        SDL_DestroyTexture(el.second);
    }
    if (layout)
        delete layout;
}

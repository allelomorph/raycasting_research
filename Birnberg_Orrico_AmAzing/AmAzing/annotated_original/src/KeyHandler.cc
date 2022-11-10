#include "KeyHandler.hh"


// TBD: why is keyStates unordered? advanteage in hash map vs binary tree?
KeyHandler::KeyHandler () {
    keyStates = {
        {SDLK_UP, SDL_RELEASED},
        {SDLK_DOWN, SDL_RELEASED},
        {SDLK_LEFT, SDL_RELEASED},
        {SDLK_RIGHT, SDL_RELEASED},
        {SDLK_a, SDL_RELEASED},
        {SDLK_d, SDL_RELEASED},
        {SDLK_f, SDL_RELEASED},
        {SDLK_m, SDL_RELEASED},
        {SDLK_p, SDL_RELEASED},
        {SDLK_q, SDL_RELEASED},
        {SDLK_ESCAPE, SDL_RELEASED}
    };
}

bool KeyHandler::isPressed(SDL_Keycode keysym) {
    return(keyStates[keysym] == SDL_PRESSED);
}

bool KeyHandler::isReleased(SDL_Keycode keysym) {
    return(keyStates[keysym] == SDL_RELEASED);
}

void KeyHandler::handleKeyEvent(SDL_KeyboardEvent& e) {
    // unrecognized key
    if (keyStates.find(e.keysym.sym) == keyStates.end())
        return;
    // most keys updated live to match keyboard input
    // f, m, and p instead toggle their down state for every user keypress
    // TBD: this is redundant when they already set showFPS, showMap, and call
    //   Mix_(Pause|Resume)Music, respectively
    if (e.keysym.sym == SDLK_f || e.keysym.sym == SDLK_m ||
        e.keysym.sym == SDLK_p) {
        if (e.type == SDL_KEYDOWN)
            keyStates[e.keysym.sym] = !keyStates[e.keysym.sym];
    } else {
        keyStates[e.keysym.sym] = e.state;
    }
}

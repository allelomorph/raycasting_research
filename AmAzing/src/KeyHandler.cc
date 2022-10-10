#include "../include/KeyHandler.hh"

// TBD: why is keyStates unordered? is there virtue in keeping the key order as declared below?
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
    if (keyStates.find(e.keysym.sym) == keyStates.end())
        return;
    // f, m, and e only toggle a keydown state used as a boolean for their respective game settings
    // whereas all other keys are updated live as pressed by player
    // TBD: what are f/m/e used for?
    if (!(e.keysym.sym == SDLK_f || e.keysym.sym == SDLK_m || e.keysym.sym == SDLK_p))
        keyStates[e.keysym.sym] = e.state;
    else if (e.type == SDL_KEYDOWN)
        keyStates[e.keysym.sym] = !keyStates[e.keysym.sym];
}

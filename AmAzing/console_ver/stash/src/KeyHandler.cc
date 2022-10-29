#include "../include/KeyHandler.hh"

#include <iostream>  // debug only

// TBD: why is keyStates unordered? is there virtue in keeping the (map) key order as declared below?
// TBD: why not greb key states from SDL itself, and then use State booleans for toggles?
KeyHandler::KeyHandler () {
    key_states = {
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
    return(key_states[keysym] == SDL_PRESSED);
}

bool KeyHandler::isReleased(SDL_Keycode keysym) {
    return(key_states[keysym] == SDL_RELEASED);
}

void KeyHandler::handleKeyEvent(SDL_KeyboardEvent& e) {
    std::cout << "\tkeysym: '" << e.keysym.sym << " event: " <<
        (e.type == SDL_KEYDOWN ? "SDL_KEYDOWN" : "SDL_KEYUP") << '\n';
    if (key_states.find(e.keysym.sym) == key_states.end())
        return;
    // f (toggle FPS counter,) m (toggle map overlay,) and p (pause music)
    //   toggle states stored as key_states
    // all other key states are updated live as pressed by player
    if (!(e.keysym.sym == SDLK_f || e.keysym.sym == SDLK_m || e.keysym.sym == SDLK_p))
        key_states[e.keysym.sym] = e.state;
    else if (e.type == SDL_KEYDOWN)
        key_states[e.keysym.sym] = !key_states[e.keysym.sym];
    std::cout << "amazing keystates at end of frame:\n";
    for (const auto pair : key_states) {
        std::cout << "\t'" << pair.first << ": " <<
            (pair.second == SDL_PRESSED ? "SDL_PRESSED" : "SDL_RELEASED") << '\n';
    }
}

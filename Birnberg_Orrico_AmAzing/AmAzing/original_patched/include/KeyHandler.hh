#ifndef KEYHANDLER_HH
#define KEYHANDLER_HH

#include <unordered_map>
#include <SDL2/SDL.h>

struct KeyHandler {
    std::unordered_map<SDL_Keycode, uint8_t> keyStates;

    bool isPressed(SDL_Keycode keysym);
    bool isReleased(SDL_Keycode keysym);
    void handleKeyEvent(SDL_KeyboardEvent &e);

    KeyHandler();
};

#endif  // KEYHANDLER_HH

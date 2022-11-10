#ifndef KEYHANDLER_HH
#define KEYHANDLER_HH


#include <SDL2/SDL.h>     // SDL_Keycode SDL_KeyboardEvent

#include <cstdint>        // uint8_t

#include <unordered_map>


class KeyHandler {
private:
    std::unordered_map<SDL_Keycode, uint8_t> keyStates;

public:
    KeyHandler();

    bool isPressed(SDL_Keycode keysym);
    bool isReleased(SDL_Keycode keysym);
    void handleKeyEvent(SDL_KeyboardEvent &e);
};


#endif  // KEYHANDLER_HH

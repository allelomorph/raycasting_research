#ifndef KEYHANDLER_HH
#define KEYHANDLER_HH

#include <unordered_map>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_events.h>  // SDL_KeyboardEvent

struct KeyHandler {
    std::unordered_map<SDL_Keycode, uint8_t> key_states;

    bool isPressed(SDL_Keycode keysym);
    bool isReleased(SDL_Keycode keysym);
    void handleKeyEvent(SDL_KeyboardEvent& e);

    KeyHandler();
};

#endif  // KEYHANDLER_HH

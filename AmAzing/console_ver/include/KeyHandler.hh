#ifndef KEYHANDLER_HH
#define KEYHANDLER_HH

#include <unordered_map>

// TBD: how to grab keypresses from console? (see ANSI escape codes and ncurses)
struct KeyHandler {
    std::unordered_map</*SDL_Keycode*/, uint8_t> keyStates;

    bool isPressed(/*SDL_Keycode*/ keysym);
    bool isReleased(/*SDL_Keycode*/ keysym);
    void handleKeyEvent(/*SDL_KeyboardEvent*/ &e);

    KeyHandler();
};

#endif  // KEYHANDLER_HH

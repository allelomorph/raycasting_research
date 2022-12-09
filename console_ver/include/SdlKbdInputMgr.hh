#ifndef SDLKBDINPUTMGR_HH
#define SDLKBDINPUTMGR_HH

#include "KeyState.hh"

#include <unordered_map>


class SdlKbdInputMgr {
private:
    std::unordered_map<SDL_Keycode, SdlKeyState> key_states;

public:
    SdlKbdInputMgr();

    // LinuxKbdInputMgr::consumeKeyEvents gets its events from a source that
    //   only provides keyboard events, so it can loop through them on its own.
    //   However, SDL_PollEvent returns events of all types, so instead with SDL
    //   we have a function to consume individual key events from the mixed list.
    void consumeKeyEvent(const SDL_KeyboardEvent& ev);

    bool keyDownThisFrame(const SDL_Keycode keysym);
    bool isPressed(const SDL_Keycode keysym);
    bool isReleased(const SDL_Keycode keysym);
    // Game frames may pass between consuming a key press event and its first
    //   autorepeat event; so to maintain the distinction between a key being
    //   pressed and a key being held down at frame granularity, this can be
    //   used to mark any keys pressed this frame as held (repeat) at the end
    //   of that frame
    void decayToAutorepeat();
};


#endif  // SDLKBDINPUTMGR_HH

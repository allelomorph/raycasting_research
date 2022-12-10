#include "SdlKbdInputMgr.hh"


SdlKbdInputMgr::SdlKbdInputMgr() {
    key_states = std::unordered_map<SDL_Keycode, SdlKeyState> {
        { SDLK_LEFT,   SdlKeyState (SDLK_LEFT   ) },
        { SDLK_UP,     SdlKeyState (SDLK_UP     ) },
        { SDLK_RIGHT,  SdlKeyState (SDLK_RIGHT  ) },
        { SDLK_DOWN,   SdlKeyState (SDLK_DOWN   ) },
        { SDLK_c,      SdlKeyState (SDLK_c      ) },
        { SDLK_F1,     SdlKeyState (SDLK_F1     ) },
        { SDLK_F2,     SdlKeyState (SDLK_F2     ) },
        { SDLK_F3,     SdlKeyState (SDLK_F3     ) },
        { SDLK_F4,     SdlKeyState (SDLK_F4     ) },
        { SDLK_F5,     SdlKeyState (SDLK_F5     ) },
        { SDLK_LSHIFT, SdlKeyState (SDLK_LSHIFT ) },
        { SDLK_RSHIFT, SdlKeyState (SDLK_RSHIFT ) },
        { SDLK_LCTRL,  SdlKeyState (SDLK_LCTRL  ) },
        { SDLK_RCTRL,  SdlKeyState (SDLK_RCTRL  ) },
        { SDLK_LALT,   SdlKeyState (SDLK_LALT   ) },
        { SDLK_RALT,   SdlKeyState (SDLK_RALT   ) },
        { SDLK_ESCAPE, SdlKeyState (SDLK_ESCAPE ) }
    };
}

bool SdlKbdInputMgr::keyDownThisFrame(const SDL_Keycode keysym) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.keyDownThisFrame());
    return false;
}

bool SdlKbdInputMgr::isPressed(const SDL_Keycode keysym) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.isPressed());
    return false;
}

bool SdlKbdInputMgr::isReleased(const SDL_Keycode keysym) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.isReleased());
    return true;
}

// SDL_event.key or SDL_KeyboardEvent
//   .state (Uint8)
//   .repeat (Uint8)
//   .keysym (SDL_Keysym)
//      .sym (SDL_Keycode (Sint32))
void SdlKbdInputMgr::consumeKeyEvent(const SDL_KeyboardEvent& ev) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(ev.keysym.sym) };
    if (ks_it != key_states.end())
        ks_it->second.update(ev.state);
}

void SdlKbdInputMgr::decayToAutorepeat() {
    for (auto& pair : key_states) {
        pair.second.decayToAutorepeat();
    }
}

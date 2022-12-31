#include "SdlKbdInputMgr.hh"

#include <SDL2/SDL_keycode.h>  // SDL_Keycode SDLK_*


SdlKbdInputMgr::SdlKbdInputMgr() {
    key_states = std::unordered_map<int32_t /*SDL_Keycode*/, KeyState> {
        { SDLK_LEFT,   KeyState (SDLK_LEFT   ) },
        { SDLK_UP,     KeyState (SDLK_UP     ) },
        { SDLK_RIGHT,  KeyState (SDLK_RIGHT  ) },
        { SDLK_DOWN,   KeyState (SDLK_DOWN   ) },
        { SDLK_c,      KeyState (SDLK_c      ) },
        { SDLK_F1,     KeyState (SDLK_F1     ) },
        { SDLK_F2,     KeyState (SDLK_F2     ) },
        { SDLK_F3,     KeyState (SDLK_F3     ) },
        { SDLK_F4,     KeyState (SDLK_F4     ) },
        { SDLK_F5,     KeyState (SDLK_F5     ) },
        { SDLK_LSHIFT, KeyState (SDLK_LSHIFT ) },
        { SDLK_RSHIFT, KeyState (SDLK_RSHIFT ) },
        { SDLK_LCTRL,  KeyState (SDLK_LCTRL  ) },
        { SDLK_RCTRL,  KeyState (SDLK_RCTRL  ) },
        { SDLK_LALT,   KeyState (SDLK_LALT   ) },
        { SDLK_RALT,   KeyState (SDLK_RALT   ) },
        { SDLK_ESCAPE, KeyState (SDLK_ESCAPE ) }
    };
}

// SDL_event.key / SDL_KeyboardEvent
//     .state (Uint8)
//     .repeat (Uint8)
//     .keysym (SDL_Keysym)
//         .scancode (SDL_Scancode (enum(int)))
//         .sym (SDL_Keycode (Sint32))
//         .mod (Uint16)
//         .unused (Uint32)
void SdlKbdInputMgr::consumeKeyEvent(const SDL_KeyboardEvent& ev) {
    // remember that operator[] inserts a new key if not found
    auto ks_it { key_states.find(ev.keysym.sym) };
    if (ks_it != key_states.end())
        ks_it->second.update(ev.state);
}

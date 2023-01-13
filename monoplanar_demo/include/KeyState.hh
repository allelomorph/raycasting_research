#ifndef KEYSTATE_HH
#define KEYSTATE_HH

#include <cstdint>              // int32_t


// See linux/input.h for struct input_event
// See SDL2/SDL_events.h for SDL_RELEASED and SDL_PRESSED
enum KeyValue { Release,       // input_event.value release,    SDL_RELEASED
                Press,         // input_event.value press,      SDL_PRESSED
                Autorepeat };  // input_event.value autorepeat,
                               //   (SDL uses SDL_KeyboardEvent.repeat as separate flag)

class KeyState {
protected:
    bool pressed { false };  // key is down
    bool repeat  { false };  // key down event was in earlier frame

public:
    // input_event.code (__u16) or SDL_keycode (Sint32)
    // (SDL_keycode enum range may not require a 4-byte int, but also uses
    //   bitmask SDLK_SCANCODE_MASK (1<<30), so we keep it 4 bytes)
    int32_t code;

    // KeyState() only needed if map operator[] called with missing key,
    //   which should not happen, so deleting it acts like an assert
    KeyState() = delete;
    KeyState(const int32_t c) : code(c) {};

    // input_event.value (__s32) or SDL_KeyboardEvent.state (Uint8)
    // int size throttled to 1 byte due to only handling KeyValue values
    // Not typing state as KeyValue to allow implicit conversion from __s32
    void update(const uint8_t state) {
        if (state < KeyValue::Autorepeat) {
            repeat = false;
            pressed = static_cast<bool>(state);
        }
    }

    inline bool keyDownThisFrame() const {
        return (pressed && !repeat);
    }

    inline bool isPressed() const {
        return (pressed);
    }

    inline bool isReleased() const {
        return (!pressed);
    }

    // Game frames may pass between consuming a key press event and its first
    //   autorepeat event; so to maintain the distinction between a key being
    //   pressed and a key being held down at frame granularity, this can be
    //   used to mark any keys pressed this frame as held (repeat) at the end
    //   of that frame
    inline void decayToAutorepeat() {
        if (pressed)
            repeat = true;
    }
};


#endif  // KEYSTATE_HH

#ifndef KEYSTATE_HH
#define KEYSTATE_HH


#include <SDL2/SDL_keycode.h>  // SDL_Keycode

#include <cstdint>              // uint16_t


class KeyState {
protected:
    bool pressed { false };  // key is down
    bool repeat  { false };  // key down event was in earlier frame

public:
    // KeyState() needed in calls to child class constructors

    inline bool keyDownThisFrame() {
        return (pressed && !repeat);
    }

    inline bool isPressed() {
        return (pressed);
    }

    inline bool isReleased() {
        return (!pressed);
    }

    inline void decayToAutorepeat() {
        if (pressed)
            repeat = true;
    }
};

// input_event.code is __u16 in /linux/input.h, should be KEY_*, see /linux/input-event-codes.h
using LinuxKeyCode = uint16_t;
// input_event.value is __s32 in /linux/input.h
enum class LinuxKeyValue { Release, Press, Autorepeat };

class LinuxKeyState : public KeyState {
public:
    LinuxKeyCode code;

    // LinuxKeyState() only needed if map operator[] called with missing key,
    //   which should not happen, so deleting it acts like an assert
    LinuxKeyState() = delete;
    LinuxKeyState(const LinuxKeyCode c) : code(c) {};

    inline void update(const LinuxKeyValue value) {
        if (value < LinuxKeyValue::Autorepeat) {
            repeat = false;
            pressed = static_cast<bool>(value);
        }
    }
};

class SdlKeyState : public KeyState {
public:
    SDL_Keycode code;

    // SdlKeyState() only needed if map operator[] called with missing key,
    //   which should not happen, so deleting it acts like an assert
    SdlKeyState() = delete;
    SdlKeyState(const SDL_Keycode c) : code(c) {};

    // SDL_KeyboardEvent.state (SDL Uint8 equivalent to uint8_t)
    // SDL_RELEASED or SDL_PRESSED, from SDL2/SDL_events.h
    inline void update(const Uint8 value) {
        if (value < 2) {
            repeat = false;
            pressed = static_cast<bool>(value);
        }
    }
};


#endif  // KEYSTATE_HH

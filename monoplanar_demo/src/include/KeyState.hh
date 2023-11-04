#ifndef KEYSTATE_HH
#define KEYSTATE_HH

#include <cstdint>              // int32_t


// Release    : input_event.value = 0 : SDL_KeyboardEvent.state = SDL_RELEASED
// Press      : input_event.value = 1 : SDL_KeyboardEvent.state = SDL_PRESSED
// Autorepeat : input_event.value = 2 : SDL_KeyboardEvent.repeat = 1
// (See linux/input.h for input_event; SDL2/SDL_events.h for SDL_KeyboardEvent)
enum KeyValue { Release, Press, Autorepeat };

class KeyState {
protected:
    bool pressed { false };  // key is down
    bool repeat  { false };  // key down event was in earlier frame

public:
    // keysym code, could be input_event.code (__u16) or SDL_keycode (Sint32)
    // (SDL_keycode enum range may not require a 4-byte int, but also uses
    //   bitmask SDLK_SCANCODE_MASK (1<<30), so we keep it 4 bytes)
    int32_t code;

    // KeyState() only needed if map operator[] called with missing key
    KeyState() = delete;
    KeyState(const int32_t c) : code(c) {};

    /**
     * @brief align key state with input
     *
     * @param state - new key state; could be input_event.value (__s32) or
     *   SDL_KeyboardEvent.state (Uint8) (narrowed to 1 byte due to values
     *   being 0-2, but uint8_t rather than KeyValue to allow implicit
     *   conversion from __s32)
     */
    void update(const uint8_t state) {
        if (state < KeyValue::Autorepeat) {
            repeat = false;
            pressed = static_cast<bool>(state);
        }
    }
    /**
     * @brief test if key was marked as pressed on this game frame
     *
     * @return true if pressed and not autorepreat
     */
    inline bool keyDownThisFrame() const {
        return (pressed && !repeat);
    }
    /**
     * @brief test if key is currently pressed
     *
     * @return true if key is down
     */
    inline bool isPressed() const {
        return (pressed);
    }
    /**
     * @brief test if key is currently not pressed
     *
     * @return true if key is not down
     */
    inline bool isReleased() const {
        return (!pressed);
    }
    /**
     * @brief convert key press event from current game frame to autorepeat
     *
     * @notes Game frames may pass between consuming a key press event and its
     *   first autorepeat event; so to maintain the distinction between a key
     *   being pressed and a key being held down at frame granularity, this
     *   can be used to mark any keys pressed this frame as held, or
     *   repeating, at the end of that frame.
     */
    inline void decayToAutorepeat() {
        if (pressed)
            repeat = true;
    }
};


#endif  // KEYSTATE_HH

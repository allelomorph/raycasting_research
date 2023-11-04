#ifndef KBDINPUTMGR_HH
#define KBDINPUTMGR_HH

#include "KeyState.hh"

#include <SDL2/SDL_events.h>  // SDL_KeyboardEvent

#include <unordered_map>


// polymorphic interface to LinuxKbdInputMgr, SdlKbdInputMgr
class KbdInputMgr {
protected:
    // map keys are either input_event.code (__u16, see linux/input.h,) or
    //   SDL_Keycode (Sint32, see SDL2/SDL_keycode.h)
    std::unordered_map<int32_t, KeyState> key_states;

public:
    // polymorphic classes need virtual dtor
    virtual ~KbdInputMgr() {}

    /**
     * @brief Intended to be overidden in LinuxKbdInputMgr, where all of the
     *   accumulated linux keyboard events can be consumed in one function call.
     */
    virtual void consumeKeyEvents() {}
    /**
     * @brief Intended to be overidden in SdlKbdInputMgr, where keyboard events
     *   are mixed with other SDL event types in a single queue, and so it is
     *   preferable to have a function which consumes a single keyboard event
     *   when it is polled from the queue.
     */
    virtual void consumeKeyEvent(const SDL_KeyboardEvent& /*ev*/) {}
    /**
     * @brief test if key was pressed on this game frame
     *
     * @param code - keysym, either input_event.code or SDL_Keycode
     *
     * @return true if pressed and not autorepreat
     */
    bool keyDownThisFrame(const int32_t code) const;
    /**
     * @brief test if key is currently pressed
     *
     * @param code - keysym, either input_event.code or SDL_Keycode
     *
     * @return true if key is down
     */
    bool isPressed(const int32_t code) const;
    /**
     * @brief test if key is currently not pressed
     *
     * @param code - keysym, either input_event.code or SDL_Keycode
     *
     * @return true if key is not down
     */
    bool isReleased(const int32_t code) const;
    /**
     * @brief convert key press event from current game frame to autorepeat
     *
     * @notes Game frames may pass between consuming a key press event and its
     *   first autorepeat event; so to maintain the distinction between a key
     *   being pressed and a key being held down at frame granularity, this
     *   can be used to mark any keys pressed this frame as held, or
     *   repeating, at the end of that frame.
     */
    void decayToAutorepeat();
};


#endif  // KBDINPUTMGR_HH

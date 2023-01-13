#ifndef KBDINPUTMGR_HH
#define KBDINPUTMGR_HH

#include "KeyState.hh"

#include <SDL2/SDL_events.h>  // SDL_KeyboardEvent

#include <unordered_map>


// polymorphic interface to LinuxKbdInputMgr, SdlKbdInputMgr
class KbdInputMgr {
protected:
    // map keys are either linux/input.h:input_event.code (__u16) or SDL_keycode (Sint32)
    std::unordered_map<int32_t, KeyState> key_states;

public:
    // polymorphic classes need virtual dtor
    virtual ~KbdInputMgr() {}

    // TBD: Having both consume event functions here is an unresolved
    //   design problem in trying to make a shared interface for both
    //   SDL and Linux keyboard events. Currently handled by having empty
    //   definitions here with only the relevant function overridden in child
    //   classes.

    // Linux events are stored in separate virtual files based on type, so we
    //   can consume all the keyboard events in one function call. SDL events
    //   of all types are instead stored together in one shared queue, so it is
    //   is preferable to have a function which consumes a single keyboard event
    //   when it is polled from the queue.
    virtual void consumeKeyEvents() {}
    virtual void consumeKeyEvent(const SDL_KeyboardEvent& /*ev*/) {}

    bool keyDownThisFrame(const int32_t code) const;
    bool isPressed(const int32_t code) const;
    bool isReleased(const int32_t code) const;

    // Game frames may pass between consuming a key press event and its first
    //   autorepeat event; so to maintain the distinction between a key being
    //   pressed and a key being held down at frame granularity, this can be
    //   used to mark any keys pressed this frame as held (repeating) at the
    //   end of that frame
    void decayToAutorepeat();
};


#endif  // KBDINPUTMGR_HH

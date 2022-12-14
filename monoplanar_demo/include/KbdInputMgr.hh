#ifndef KBDINPUTMGR_HH
#define KBDINPUTMGR_HH

#include "KeyState.hh"

#include <SDL2/SDL_events.h>  // SDL_KeyboardEvent

#include <unordered_map>


// polymorphic interface to LinuxKbdInputMgr, SdlKbdInputMgr
class KbdInputMgr {
protected:
    // map keys are either linux/input.h:input_event.code or SDL_keycode
    std::unordered_map<int32_t, KeyState> key_states;

public:
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

    virtual bool keyDownThisFrame(const int32_t code) = 0;
    virtual bool isPressed(const int32_t code) = 0;
    virtual bool isReleased(const int32_t code) = 0;

    virtual void decayToAutorepeat() = 0;
};


#endif  // KBDINPUTMGR_HH

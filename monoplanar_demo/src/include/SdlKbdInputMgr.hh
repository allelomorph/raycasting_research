#ifndef SDLKBDINPUTMGR_HH
#define SDLKBDINPUTMGR_HH

#include "KbdInputMgr.hh"

#include <cstdint>         // int32_t

class SdlKbdInputMgr : public KbdInputMgr {
public:
    SdlKbdInputMgr();

    // See notes on consume key functions in parent class
    void consumeKeyEvent(const SDL_KeyboardEvent& ev);
};


#endif  // SDLKBDINPUTMGR_HH

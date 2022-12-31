#ifndef APP_HH
#define APP_HH

#include "FpsCalc.hh"
#include "Settings.hh"           // TtyDisplayMode
#include "KbdInputMgr.hh"
#include "DdaRaycastEngine.hh"
// #include "DisplayMgr.hh"
#include "TtyDisplayMgr.hh"

#include <csignal>               // sig_atomic_t
#include <cstdint>               // uint16_t
#include <cassert>               // uint16_t

#include <string>
#include <vector>
#include <memory>                // unique_ptr


extern volatile std::sig_atomic_t sigint_sigterm_received;
extern volatile std::sig_atomic_t sigwinch_received;

class App {
public:
    App() = delete;
    App(const char* efn, const std::string& mfn, const bool _tty_io,
        TtyDisplayMode tty_display_mode);
    ~App();

    void run();

private:
    std::string exec_filename;
    std::string map_filename;

    // operation flags
    //
    bool    tty_io   { true };
    bool    stop     { false };
    bool    pause    { false };

    // frame timing
    //
    ProcessTimeFpsCalc           pt_fps_calc;
    RealTimeFpsCalc              rt_fps_calc;

    // user-adjustable game settings
    //
    Settings                     settings;

    // user input
    //
    // pointer for polymorphism with Linux and SDL mode child classes
    std::unique_ptr<KbdInputMgr> kbd_input_mgr { nullptr };

    // raycasting
    //
    DdaRaycastEngine             raycast_engine;

    // display
    //
    TtyDisplayMgr                display_mgr;

    void initialize();

    void getEvents();

    // TBD: while kbd_input_mgr is polymorphic, given that there is no alignment
    //   between SDLK_* and KEY_* keycode values, short of creating isPressed()
    //   functions for every supported key, there need to be separate
    //   updateFromInput functions to check the different keysym sets
    // TBD: also, while it is another addition to per-frame computation to check
    //   tty_io every call of updateFromInput, this still seems preferable to
    //   checking tty_io once to set a function pointer `void (App::*fp)();` to
    //   updateFromLinuxInput or updateFromSdlInput, as every time fp is called,
    //   it would require THREE pointer dereferences: `(this->*(this->fp))()`, see:
    //   https://stackoverflow.com/questions/2402579/function-pointer-to-member-function
    void updateFromInput();
    void updateFromLinuxInput();
    void updateFromSdlInput();
};


#endif  // APP_HH

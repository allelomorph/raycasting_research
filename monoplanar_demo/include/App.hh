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
        TtyDisplayMode tty_display_mode) : exec_filename(efn), map_filename(mfn),
                                           tty_io(_tty_io) {
        if (tty_io)
            settings.tty_display_mode = tty_display_mode;
    }

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
    void updateState();
};


#endif  // APP_HH

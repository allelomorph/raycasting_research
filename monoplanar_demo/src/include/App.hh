#ifndef APP_HH
#define APP_HH

#include "FpsCalc.hh"
#include "Settings.hh"           // TtyDisplayMode
#include "KbdInputMgr.hh"
#include "DdaRaycastEngine.hh"
#include "WindowMgr.hh"
#include "TtyWindowMgr.hh"
#include "SdlWindowMgr.hh"

#include <csignal>               // sig_atomic_t
#include <cstdint>               // uint16_t

#include <string>
#include <memory>                // unique_ptr


// global to be visible to sigaction
extern volatile std::sig_atomic_t sigint_sigterm_received;
extern volatile std::sig_atomic_t sigwinch_received;

class App {
public:
    App() = delete;
    App(const char* efn, const std::string& mfn, const bool _tty_io,
        TtyDisplayMode tty_display_mode);
    ~App();

    /**
     * @brief Main game loop
     */
    void run();

private:
    std::string exec_filename;
    std::string map_filename;

    // operation flags
    //
    bool    tty_io   { true };
    bool    stop     { false };

    // frame timing
    //
    ProcessTimeFpsCalc           pt_fps_calc;
    RealTimeFpsCalc              rt_fps_calc;

    // user-adjustable game settings
    //
    Settings                     settings;

    // user input
    //
    // polymorphic pointer to LinuxKbdInputMgr and SdlKbdInputMgr
    std::unique_ptr<KbdInputMgr> kbd_input_mgr { nullptr };

    // raycasting
    //
    DdaRaycastEngine             raycast_engine;

    // video output
    //
    // polymorphic pointer to LinuxWindowMgr and SdlWindowMgr
    std::unique_ptr<WindowMgr>   window_mgr;

    /**
     * @brief Setup of main game loop
     */
    void initialize();
    /**
     * @brief Collect user input and other changing conditions relevant to game
     *   operation
     */
    void getEvents();
    /**
     * @brief Select update function based on display mode
     *
     * @notes kbd_input_mgr is polymorphic, but given that there is no agreement
     *     between SDLK_* and KEY_* keycode values, short of creating isPressed()
     *     functions for every supported key, there need to be separate
     *     updateFromInput functions to check the different keysym sets.
     *   Checking tty_io in every frame seems preferable to setting a function
     *     pointer, eg `void (App::*fp)();` to updateFromLinuxInput or
     *     updateFromSdlInput, as every time fp was called, it would require
     *     THREE pointer dereferences: `(this->*(this->fp))()`, see:
     *     https://stackoverflow.com/questions/2402579/function-pointer-to-member-function
     */
    void updateFromInput();
    /**
     * @brief Update player position, direction, and game settings from Linux
     *   input devices
     */
    void updateFromLinuxInput();
    /**
     * @brief Update player position, direction, and game settings from SDL
     *   device input
     */
    void updateFromSdlInput();
};


#endif  // APP_HH

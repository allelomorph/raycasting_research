#include "App.hh"
#include "safeCExec.hh"      // C_*
#include "Xterm.hh"          // CtrlSeqs
#include "LinuxKbdInputMgr.hh"
//#include "SdlKbdInputMgr.hh"

#include <csignal>           // sigaction SIG* sig_atomic_t
#include <cstring>           // memset


// Need to be global to be visible to sigaction
volatile std::sig_atomic_t sigint_sigterm_received = 0;
volatile std::sig_atomic_t sigwinch_received = 0;

static void sigint_sigterm_handler(int /*signal*/) {
    sigint_sigterm_received = 1;
}

static void sigwinch_handler(int /*signal*/) {
    sigwinch_received = 1;
}

void App::initialize() {
    pt_fps_calc.initialize();
    rt_fps_calc.initialize();

    struct sigaction sa;
    // valgrind complains if struct is uninitialized
    // struct definition is implementation-dependent, so no brace initializer
    // memset has no return or errno, so no safeCExec
    std::memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigint_sigterm_handler;
    safeCExec(sigaction, "sigaction", C_RETURN_TEST(int, (ret == -1)),
              SIGINT, &sa, nullptr);
    safeCExec(sigaction, "sigaction", C_RETURN_TEST(int, (ret == -1)),
              SIGTERM, &sa, nullptr);
    std::memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigwinch_handler;
    safeCExec(sigaction, "sigaction", C_RETURN_TEST(int, (ret == -1)),
              SIGWINCH, &sa, nullptr);

    display_mgr.initialize(settings);
    raycast_engine.updateScreenSize(display_mgr.screenWidth());
    // parse map file to get maze and starting actor positions
    raycast_engine.loadMapFile(map_filename);
/*
    kbd_input_mgr = tty_io ?
        std::unique_ptr<LinuxKbdInputMgr>(new LinuxKbdInputMgr(exec_filename)) :
        std::unique_ptr<SdlKbdInputMgr>(new SdlKbdInputMgr());
*/
    kbd_input_mgr = std::unique_ptr<LinuxKbdInputMgr>(
        new LinuxKbdInputMgr(exec_filename) );
}

void App::run() {
    initialize();

    // TBD: better consolidate these two tests
    while(!sigint_sigterm_received && !stop) {
        pt_fps_calc.calculate();
        rt_fps_calc.calculate();

        getEvents();
        updateState();

        if (tty_io && sigwinch_received) {
            display_mgr.clearDisplay();
            // terminal window size changes require rehiding the cursor
            std::cout << Xterm::CtrlSeqs::HideCursor();

            display_mgr.fitToWindow(settings.map_proportion);
            raycast_engine.updateScreenSize(display_mgr.screenWidth());

            sigwinch_received = 0;
        }

        raycast_engine.castRays(settings);

        display_mgr.renderView(raycast_engine.fov_rays);
        display_mgr.renderMap(raycast_engine);
        display_mgr.renderHUD(pt_fps_calc.frame_duration_mvg_avg,
                              rt_fps_calc.frame_duration_mvg_avg.count(),
                              settings, raycast_engine, kbd_input_mgr);

        // TBD: debug errors on terminal window size changes
        if (tty_io && sigwinch_received)
            continue;

        display_mgr.drawScreen(settings);
    }

    if (tty_io) {
        display_mgr.clearDisplay();
        std::cout << Xterm::CtrlSeqs::ShowCursor();
    }
}

void App::getEvents() {
    if (tty_io) {
        kbd_input_mgr->consumeKeyEvents();
    } else {
    }
}

void App::updateState() {
    // ctrl+c: simulate SIGINT (quit)
    if ((kbd_input_mgr->isPressed(KEY_LEFTCTRL) ||
         kbd_input_mgr->isPressed(KEY_RIGHTCTRL)) &&
        kbd_input_mgr->isPressed(KEY_C)) {
        stop = true;
        return;
    }

    // escape key: quit
    if (kbd_input_mgr->isPressed(KEY_ESC)) {
        stop = true;
        return;
    }

    double move_speed { pt_fps_calc.frame_duration_mvg_avg * settings.base_movement_rate };
    double rot_speed  { pt_fps_calc.frame_duration_mvg_avg *
                        settings.base_movement_rate * settings.turn_rate };

    // Movement in the 2d map grid has been set up so +y/+i with map[i][j]/up on a
    //   printed map all represent moving north

    // shift key: run
    if (kbd_input_mgr->isPressed(KEY_LEFTSHIFT) ||
        kbd_input_mgr->isPressed(KEY_RIGHTSHIFT)) {
        move_speed *= 2;
        rot_speed *= 2;
    }

    // up arrow key: move forward
    if (kbd_input_mgr->isPressed(KEY_UP))
        raycast_engine.playerMoveFwd(move_speed);

    // down arrow key: move backward
    if (kbd_input_mgr->isPressed(KEY_DOWN))
        raycast_engine.playerMoveBack(move_speed);

    if (kbd_input_mgr->isPressed(KEY_LEFT)) {
        if (kbd_input_mgr->isPressed(KEY_LEFTALT) ||
            kbd_input_mgr->isPressed(KEY_RIGHTALT)) {
            // alt + left arrow key: move left (strafe)
            raycast_engine.playerStrafeLeft(move_speed);
        } else {
            // left arrow key: rotate left (CCW)
            raycast_engine.playerTurnLeft(rot_speed);
        }
    }

    if (kbd_input_mgr->isPressed(KEY_RIGHT)) {
        if (kbd_input_mgr->isPressed(KEY_LEFTALT) ||
            kbd_input_mgr->isPressed(KEY_RIGHTALT)) {
            // alt + right arrow key: move right (strafe)
            raycast_engine.playerStrafeRight(move_speed);
        } else {
            // right arrow key: roatate right (CW)
            raycast_engine.playerTurnRight(rot_speed);
        }
    }

    // F1 key: toggle FPS overlay
    if (kbd_input_mgr->keyDownThisFrame(KEY_F1))
        settings.show_fps = !settings.show_fps;

    // F2 key: toggle map overlay
    if (kbd_input_mgr->keyDownThisFrame(KEY_F2))
        settings.show_map = !settings.show_map;

    // F3 key: toggle debug mode
    if (kbd_input_mgr->keyDownThisFrame(KEY_F3))
        settings.debug_mode = !settings.debug_mode;

    // F4 key: toggle fisheye camera mode
    if (kbd_input_mgr->keyDownThisFrame(KEY_F4))
        settings.fisheye = !settings.fisheye;

    kbd_input_mgr->decayToAutorepeat();
}

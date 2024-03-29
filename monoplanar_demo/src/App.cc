#include "App.hh"
#include "safeLibcCall.hh"      // Libc*
#include "safeSdlCall.hh"       // Sdl*
#include "xterm_ctrl_seqs.hh"   // CtrlSeqs
#include "LinuxKbdInputMgr.hh"
#include "SdlKbdInputMgr.hh"

#include <SDL2/SDL.h>         // SDL_Init SDL_Quit
#include <SDL2/SDL_events.h>  // SDL_QUIT SDL_KEY* SDL_PollEvent
#include <SDL2/SDL_video.h>   // SDL_GetWindowID SDL_WINDOWEVENT_*

#include <csignal>            // sigaction SIG* sig_atomic_t
#include <cstring>            // memset


// global to be visible to sigaction
volatile std::sig_atomic_t sigint_sigterm_received { 0 };
volatile std::sig_atomic_t sigwinch_received       { 0 };

static void sigint_sigterm_handler(int /*signal*/) {
    sigint_sigterm_received = 1;
}

static void sigwinch_handler(int /*signal*/) {
    sigwinch_received = 1;
}

App::App(const char* efn, const std::string& mfn,
         const bool _tty_io, TtyDisplayMode tty_display_mode) :
    exec_filename(efn), map_filename(mfn), tty_io(_tty_io) {
    if (tty_io)
        settings.tty_display_mode = tty_display_mode;

    // wall textures are loaded with IMG_Load even when in tty mode,
    //   with SDL subsystems needed to report errors, so init regardless
    safeSdlCall(SDL_Init, "SDL_Init",
                SdlRetTest<int>{ [](const int ret){ return (ret != 0); } },
                tty_io ? 0 : SDL_INIT_VIDEO);
}

App::~App() {
    SDL_Quit();
}

void App::initialize() {
    pt_fps_calc.initialize();
    rt_fps_calc.initialize();

    struct sigaction sa;
    // valgrind complains if struct is uninitialized
    // struct definition is implementation-dependent, so no brace initializer
    // memset has no return or errno, so no safeLibcCall
    std::memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigint_sigterm_handler;
    LibcRetTest<int> sigaction_failure_test {
        [](const int ret){ return (ret == -1); } };
    safeLibcCall(sigaction, "sigaction", sigaction_failure_test,
                 SIGINT, &sa, nullptr);
    safeLibcCall(sigaction, "sigaction", sigaction_failure_test,
                 SIGTERM, &sa, nullptr);
    // sigwinch_handler only needed in tty mode, but registered in both modes
    std::memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sigwinch_handler;
    safeLibcCall(sigaction, "sigaction", sigaction_failure_test,
                 SIGWINCH, &sa, nullptr);

    // parse map file to get maze and starting actor positions
    raycast_engine.loadMapFile(map_filename);

    if (tty_io)
        window_mgr = std::unique_ptr<TtyWindowMgr>(new TtyWindowMgr());
    else
        window_mgr = std::unique_ptr<SdlWindowMgr>(new SdlWindowMgr());
    window_mgr->initialize(settings, raycast_engine.layout.h);

    raycast_engine.fitToWindow(tty_io, window_mgr->width(), window_mgr->height());

    if (tty_io) {
        kbd_input_mgr = std::unique_ptr<LinuxKbdInputMgr>(
            new LinuxKbdInputMgr(exec_filename));
    } else {
        kbd_input_mgr = std::unique_ptr<SdlKbdInputMgr>(new SdlKbdInputMgr());
    }
}

void App::run() {
    initialize();

    while(!sigint_sigterm_received && !stop) {
        pt_fps_calc.calculate();
        rt_fps_calc.calculate();

        getEvents();
        updateFromInput();

        if (tty_io && sigwinch_received) {
            window_mgr->drawEmptyFrame();
            // terminal window size changes require rehiding the cursor
            std::cout << Xterm::CtrlSeqs::HideCursor();

            window_mgr->fitToWindow(settings.map_proportion,
                                    raycast_engine.layout.h);
            raycast_engine.fitToWindow(tty_io, window_mgr->width(),
                                       window_mgr->height());

            sigwinch_received = 0;
        }

        raycast_engine.castRays(settings);

        window_mgr->renderView(raycast_engine.fov_rays, settings);
        if (settings.show_map)
            window_mgr->renderMap(raycast_engine);
        window_mgr->renderHud(pt_fps_calc.frame_duration_mvg_avg,
                              rt_fps_calc.frame_duration_mvg_avg.count(),
                              settings, raycast_engine, kbd_input_mgr.get());

        // second opportunity to abort drawing frame if SIGWINCH received
        //   during casting/rendering
        if (tty_io && sigwinch_received)
            continue;

        window_mgr->drawFrame(settings);
    }

    if (tty_io) {
        window_mgr->drawEmptyFrame();
        std::cout << Xterm::CtrlSeqs::ShowCursor();
    }
}

void App::getEvents() {
    if (tty_io) {
        kbd_input_mgr->consumeKeyEvents();
    } else {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_QUIT:
                stop = true;
                break;
            case SDL_WINDOWEVENT:
                // SDL_AddEventWatch not appropriate for window size changes
                //   due to filter function's possible execution in a separate
                //   thread: modification of the display buffer size asynchronous
                //   to pixel getting/setting would likely cause segfaults
                if (e.window.event == SDL_WINDOWEVENT_RESIZED &&
                    e.window.windowID == window_mgr->id() ) {
                    window_mgr->fitToWindow(settings.map_proportion,
                                           raycast_engine.layout.h);
                    raycast_engine.fitToWindow(tty_io, window_mgr->width(),
                                               window_mgr->height());
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                kbd_input_mgr->consumeKeyEvent(e.key);
                break;
            default:
                break;
            }
        }
    }
}

void App::updateFromInput() {
    if (tty_io) {
        updateFromLinuxInput();
    } else {
        updateFromSdlInput();
    }
}

void App::updateFromLinuxInput() {
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

    // F4 key: toggle euclidean camera mode
    if (kbd_input_mgr->keyDownThisFrame(KEY_F4))
        settings.euclidean = !settings.euclidean;

    // F10 key: ascii pixels in tty mode
    if (kbd_input_mgr->keyDownThisFrame(KEY_F10))
        settings.tty_display_mode = TtyDisplayMode::Ascii;

    // F11 key: 256 color pixels in tty mode
    if (kbd_input_mgr->keyDownThisFrame(KEY_F11)) {
        if (settings.tty_display_mode == TtyDisplayMode::Ascii) {
            // erase potential leftover chars
            window_mgr->resetBuffer();
        }
        settings.tty_display_mode = TtyDisplayMode::ColorCode;
    }

    // F12 key: true color pixels in tty mode
    if (kbd_input_mgr->keyDownThisFrame(KEY_F12)) {
        if (settings.tty_display_mode == TtyDisplayMode::Ascii) {
            // erase potential leftover chars
            window_mgr->resetBuffer();
        }
        settings.tty_display_mode = TtyDisplayMode::TrueColor;
    }

    kbd_input_mgr->decayToAutorepeat();
}

void App::updateFromSdlInput() {
    // ctrl+c: simulate SIGINT (quit)
    if ((kbd_input_mgr->isPressed(SDLK_LCTRL) ||
         kbd_input_mgr->isPressed(SDLK_RCTRL)) &&
        kbd_input_mgr->isPressed(SDLK_c)) {
        stop = true;
        return;
    }

    // escape key: quit
    if (kbd_input_mgr->isPressed(SDLK_ESCAPE)) {
        stop = true;
        return;
    }

    double move_speed { pt_fps_calc.frame_duration_mvg_avg * settings.base_movement_rate };
    double rot_speed  { pt_fps_calc.frame_duration_mvg_avg *
                        settings.base_movement_rate * settings.turn_rate };

    // shift key: run
    if (kbd_input_mgr->isPressed(SDLK_LSHIFT) ||
        kbd_input_mgr->isPressed(SDLK_RSHIFT)) {
        move_speed *= 2;
        rot_speed *= 2;
    }

    // up arrow key: move forward
    if (kbd_input_mgr->isPressed(SDLK_UP))
        raycast_engine.playerMoveFwd(move_speed);

    // down arrow key: move backward
    if (kbd_input_mgr->isPressed(SDLK_DOWN))
        raycast_engine.playerMoveBack(move_speed);

    if (kbd_input_mgr->isPressed(SDLK_LEFT)) {
        if (kbd_input_mgr->isPressed(SDLK_LALT) ||
            kbd_input_mgr->isPressed(SDLK_RALT)) {
            // alt + left arrow key: move left (strafe)
            raycast_engine.playerStrafeLeft(move_speed);
        } else {
            // left arrow key: rotate left (CCW)
            raycast_engine.playerTurnLeft(rot_speed);
        }
    }

    if (kbd_input_mgr->isPressed(SDLK_RIGHT)) {
        if (kbd_input_mgr->isPressed(SDLK_LALT) ||
            kbd_input_mgr->isPressed(SDLK_RALT)) {
            // alt + right arrow key: move right (strafe)
            raycast_engine.playerStrafeRight(move_speed);
        } else {
            // right arrow key: roatate right (CW)
            raycast_engine.playerTurnRight(rot_speed);
        }
    }

    // F1 key: toggle FPS overlay
    if (kbd_input_mgr->keyDownThisFrame(SDLK_F1))
        settings.show_fps = !settings.show_fps;

    // F2 key: toggle map overlay
    if (kbd_input_mgr->keyDownThisFrame(SDLK_F2))
        settings.show_map = !settings.show_map;

    // F3 key: toggle debug mode
    if (kbd_input_mgr->keyDownThisFrame(SDLK_F3))
        settings.debug_mode = !settings.debug_mode;

    // F4 key: toggle euclidean camera mode
    if (kbd_input_mgr->keyDownThisFrame(SDLK_F4))
        settings.euclidean = !settings.euclidean;

    kbd_input_mgr->decayToAutorepeat();
}

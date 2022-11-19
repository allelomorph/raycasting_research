#include "App.hh"
#include "Matrix.hh"         // Matrix2d Vector2d
#include "safeCExec.hh"      // C_*
#include "XtermCodes.hh"     // CursorUp

#include <time.h>            // clock clock_t CLOCKS_PER_SEC
#include <csignal>           // sigaction SIG* sig_atomic_t
#include <linux/input.h>     // KEY_*

#include <iostream>
#include <iomanip>           // setw
#include <string>

#include <sys/types.h>       // pid_t
#include <unistd.h>          // getpid

#include <cmath>             // sin cos

//#include <functional>        // ref see TBD in initialize


App::App(const char* efn, const char* mfn) :
    exec_filename(efn), map_filename(mfn) {
    state = State::getInstance();
}

App::~App() {
    if (state != nullptr)
        delete state;
}

// Need to be global to be visible to sigaction
volatile std::sig_atomic_t sigint_sigterm_received = 0;
volatile std::sig_atomic_t sigwinch_received = 0;

// TBD: due to getEvents->getKeyEvents blocking on select(), program will hang when killed until some input events are read in that frame
static void sigint_sigterm_handler(int /*signal*/) {
    sigint_sigterm_received = 1;
}

static void sigwinch_handler(int /*signal*/) {
    sigwinch_received = 1;
}

void App::initialize() {
    // parse map file, also sets pos
    try {
        // TBD: why std::ref to create a reference_wrapper here?
        state->layout = new Layout(map_filename, /*std::ref(*/state->pos/*)*/);
    } catch (std::runtime_error &re) {
        std::cerr << re.what() << std::endl;
        state->done = true;
        return;
    }

    state->dir << 0, 1;
    state->viewPlane << 2.0/3, 0;

    pt_fps_calc.initialize();
    rt_fps_calc.initialize();
    state->key_handler.initialize(exec_filename);

    // get terminal window size in chars
    safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
              0/*STDIN_FILENO*/, TIOCGWINSZ, &winsz);

    struct sigaction sa;
    sa.sa_handler = sigint_sigterm_handler;
    safeCExec(sigaction, "sigaction", C_RETURN_TEST(int, (ret == -1)),
              SIGINT, &sa, nullptr);
    safeCExec(sigaction, "sigaction", C_RETURN_TEST(int, (ret == -1)),
              SIGTERM, &sa, nullptr);
    sa.sa_handler = sigwinch_handler;
    safeCExec(sigaction, "sigaction", C_RETURN_TEST(int, (ret == -1)),
              SIGWINCH, &sa, nullptr);

    // force scrollback of all terminal text before frame display
    // TBD: is there a way to do this with CSIs instead of a dummy frame?
    for (uint16_t row {0}; row < winsz.ws_row; ++row) {
        std::cout << std::string(winsz.ws_col, ' ') << '\n';
    }
    std::cout <<
        XtermCodes::CursorHome() << XtermCodes::EraseLinesBelow();
}

void App::run() {
    initialize();

    // TBD: better consolidate these two tests
    while(!sigint_sigterm_received && !state->done) {
        pt_fps_calc.calculate();
        rt_fps_calc.calculate();

        getEvents();
        updateData(/*fps_calc.moving_avg_frame_time*/);

        if (sigwinch_received) {
            std::cout <<
                XtermCodes::CursorHome() << XtermCodes::EraseLinesBelow();
            safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
                      0/*STDIN_FILENO*/, TIOCGWINSZ, &winsz);
            sigwinch_received = 0;
        }
        printDebugHUD();
    }

    // erases last frame printed to terminal
    // TBD: make into function?
    std::cout <<
        XtermCodes::CursorHome() << XtermCodes::EraseLinesBelow();
}

void App::getEvents() {
    state->key_handler.getKeyEvents();
}

static Vector2d rotate2d(Vector2d vector, double rotSpeed) {
    Matrix2d rotate;
    rotate <<
        std::cos(rotSpeed), -std::sin(rotSpeed),
        std::sin(rotSpeed), std::cos(rotSpeed);
    return (rotate * vector);
}

void App::updateData() {
    // TBD: rationale for these calcs?
    double moveSpeed { pt_fps_calc.frame_duration_mvg_avg * 4 };
    double rotSpeed  { pt_fps_calc.frame_duration_mvg_avg * 2 };

    // q or escape keys: quit
    if (state->key_handler.isPressed(KEY_Q) ||
        state->key_handler.isPressed(KEY_ESC)) {
        state->done = true;
        return;
    }

    // left arrow key: rotate left
    if (state->key_handler.isPressed(KEY_LEFT)) {
        state->dir = rotate2d(state->dir, rotSpeed);
        state->viewPlane = rotate2d(state->viewPlane, rotSpeed);
    }

    // right arrow key: roatate right
    if (state->key_handler.isPressed(KEY_RIGHT)) {
        state->dir = rotate2d(state->dir, -rotSpeed);
        state->viewPlane = rotate2d(state->viewPlane, -rotSpeed);
    }

    // up arrow key: move forward
    if (state->key_handler.isPressed(KEY_UP)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) + state->dir(0) * moveSpeed)][int(state->pos(1))]) {
            state->pos(0) += state->dir(0) * moveSpeed;
        }
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(1) * moveSpeed)]) {
            state->pos(1) += state->dir(1) * moveSpeed;
        }
    }

    // down arrow key: move backward
    if (state->key_handler.isPressed(KEY_DOWN)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(0) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(0) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) - state->dir(1) * moveSpeed)])
            state->pos(1) -= state->dir(1) * moveSpeed;
    }

    // a key: move left (strafe)
    if (state->key_handler.isPressed(KEY_A)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) - state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) -= state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) + state->dir(0) * moveSpeed)])
            state->pos(1) += state->dir(0) * moveSpeed;
    }

    // d key: move right (strafe)
    if (state->key_handler.isPressed(KEY_D)) {
        double tmp = state->pos(0);
        if (!state->layout->map[int(state->pos(0) + state->dir(1) * moveSpeed)][int(state->pos(1))])
            state->pos(0) += state->dir(1) * moveSpeed;
        if (!state->layout->map[int(tmp)][int(state->pos(1) - state->dir(0) * moveSpeed)])
            state->pos(1) -= state->dir(0) * moveSpeed;
    }

    // f key: toggle FPS
    state->showFPS = state->key_handler.isPressed(KEY_F);

    // m key: toggle map overlay
    state->showMap = state->key_handler.isPressed(KEY_M);
}


void App::printDebugHUD() {
    std::cout << std::boolalpha << "State:\n";
    std::cout << "\tflags: done: " << std::setw(5) << state->done <<
        " showFPS: " << std::setw(5) << state->showFPS <<
        " showMap: " << std::setw(5) << state->showMap << '\n';
    std::cout << "\tpos: " << state->pos << " dir: " << state->dir <<
        " viewPlane: " << state->viewPlane << '\n';
    std::cout << "Terminal window size:\n\t" << winsz.ws_row << " rows " << winsz.ws_col << " columns\n";
    std::cout << "Process Time FPS:\n\t" << (1 / pt_fps_calc.frame_duration_mvg_avg) << '\n';
    std::cout << "Real Time FPS:\n\t" << (1 / rt_fps_calc.frame_duration_mvg_avg.count()) << '\n';

    for (const auto& pair : state->key_handler.key_states) {
        auto key { pair.second };
        std::cout /*<< std::setw(6)*/ << pair.second.repr << ": " << std::noboolalpha << key.isPressed() << ' ';
    }
    std::cout << '\n' << XtermCodes::CursorUp(10);
}

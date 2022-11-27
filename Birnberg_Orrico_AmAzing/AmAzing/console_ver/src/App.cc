#include "App.hh"
#include "Matrix.hh"         // Matrix2d Vector2d
#include "safeCExec.hh"      // C_*
#include "XtermCodes.hh"     // CursorUp

#include <time.h>            // clock clock_t CLOCKS_PER_SEC
#include <csignal>           // sigaction SIG* sig_atomic_t
#include <linux/input.h>     // KEY_*
#include <termios.h>         // winsize

#include <iostream>
#include <iomanip>           // setw
#include <string>

#include <sys/types.h>       // pid_t
#include <unistd.h>          // getpid

#include <cmath>             // sin cos


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
    // TBD: change to using constructor/destructor instead of new/delete, but
    //   allowing for future ability to load new maps on the fly
    // parse map file, also sets player_pos
    try {
        state->layout = new Layout(map_filename, state->player_pos);
    } catch (std::runtime_error &re) {
        std::cerr << re.what() << std::endl;
        state->done = true;
        return;
    }

    // TBD: following Lode example over AmAzing, (x, y) order
    // Here the direction vector is a bit longer than the camera plane, so the
    //   FOV will be smaller than 90° (more precisely, the FOV is
    //   2 * atan(0.66/1.0), or 66°.
    state->player_dir << -1, 0;
    state->view_plane << 0, 2.0/3;

    pt_fps_calc.initialize();
    rt_fps_calc.initialize();

    state->key_handler.initialize(exec_filename);

    // get terminal window size in chars
    struct winsize winsz;
    safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
              0/*STDIN_FILENO*/, TIOCGWINSZ, &winsz);
    tty_window_w = winsz.ws_col;
    tty_window_h = winsz.ws_row;
    screen_buffer.resize(tty_window_w * tty_window_h);

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
    std::cout << XtermCodes::CursorHome() << XtermCodes::EraseLinesBelow() <<
        XtermCodes::HideCursor();
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
            // terminal window size changes require rehiding the cursor
            std::cout << XtermCodes::CursorHome() << XtermCodes::EraseLinesBelow() <<
                XtermCodes::HideCursor();
            struct winsize winsz;
            safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
                      0/*STDIN_FILENO*/, TIOCGWINSZ, &winsz);
            tty_window_w = winsz.ws_col;
            tty_window_h = winsz.ws_row;
            screen_buffer.resize(tty_window_w * tty_window_h);
            sigwinch_received = 0;
        }
        printDebugHUD();
    }

    // erases last frame printed to terminal
    // TBD: make into function?
    std::cout <<
        XtermCodes::CursorHome() << XtermCodes::EraseLinesBelow() <<
        XtermCodes::ShowCursor();
}

void App::getEvents() {
    state->key_handler.getKeyEvents();
}

// TBD: index order of initial player_dir and view_plane values changed to match
//   Lode's example, which is the opposite of AmAzing. Will the order of Matrix
//   members in rotateVector2d and mods in updateData need to change as well?

// TBD: use Matrix constructor instead of operator<</,
static Vector2d rotateVector2d(const Vector2d& vector, const double rot_speed) {
    Matrix2d rotate;
    rotate <<
        std::cos(rot_speed), -std::sin(rot_speed),
        std::sin(rot_speed), std::cos(rot_speed);
    return (rotate * vector);
}

void App::updateData() {
    // TBD: rationale for these calcs?
    double move_speed { pt_fps_calc.frame_duration_mvg_avg * 4 };
    double rot_speed  { pt_fps_calc.frame_duration_mvg_avg * 2 };

    // q or escape keys: quit
    if (state->key_handler.isPressed(KEY_Q) ||
        state->key_handler.isPressed(KEY_ESC)) {
        state->done = true;
        return;
    }

    // left arrow key: rotate left
    if (state->key_handler.isPressed(KEY_LEFT)) {
        state->player_dir = rotateVector2d(state->player_dir, rot_speed);
        state->view_plane = rotateVector2d(state->view_plane, rot_speed);
    }

    // right arrow key: roatate right
    if (state->key_handler.isPressed(KEY_RIGHT)) {
        state->player_dir = rotateVector2d(state->player_dir, -rot_speed);
        state->view_plane = rotateVector2d(state->view_plane, -rot_speed);
    }

    // up arrow key: move forward
    if (state->key_handler.isPressed(KEY_UP)) {
        double tmp = state->player_pos(0);
        if (!state->layout->map[int(state->player_pos(0) + state->player_dir(0) * move_speed)][int(state->player_pos(1))]) {
            state->player_pos(0) += state->player_dir(0) * move_speed;
        }
        if (!state->layout->map[int(tmp)][int(state->player_pos(1) + state->player_dir(1) * move_speed)]) {
            state->player_pos(1) += state->player_dir(1) * move_speed;
        }
    }

    // down arrow key: move backward
    if (state->key_handler.isPressed(KEY_DOWN)) {
        double tmp = state->player_pos(0);
        if (!state->layout->map[int(state->player_pos(0) - state->player_dir(0) * move_speed)][int(state->player_pos(1))])
            state->player_pos(0) -= state->player_dir(0) * move_speed;
        if (!state->layout->map[int(tmp)][int(state->player_pos(1) - state->player_dir(1) * move_speed)])
            state->player_pos(1) -= state->player_dir(1) * move_speed;
    }

    // a key: move left (strafe)
    if (state->key_handler.isPressed(KEY_A)) {
        double tmp = state->player_pos(0);
        if (!state->layout->map[int(state->player_pos(0) - state->player_dir(1) * move_speed)][int(state->player_pos(1))])
            state->player_pos(0) -= state->player_dir(1) * move_speed;
        if (!state->layout->map[int(tmp)][int(state->player_pos(1) + state->player_dir(0) * move_speed)])
            state->player_pos(1) += state->player_dir(0) * move_speed;
    }

    // d key: move right (strafe)
    if (state->key_handler.isPressed(KEY_D)) {
        double tmp = state->player_pos(0);
        if (!state->layout->map[int(state->player_pos(0) + state->player_dir(1) * move_speed)][int(state->player_pos(1))])
            state->player_pos(0) += state->player_dir(1) * move_speed;
        if (!state->layout->map[int(tmp)][int(state->player_pos(1) - state->player_dir(0) * move_speed)])
            state->player_pos(1) -= state->player_dir(0) * move_speed;
    }

    // f key: toggle FPS
    state->show_fps = state->key_handler.isPressed(KEY_F);

    // m key: toggle map overlay
    state->show_map = state->key_handler.isPressed(KEY_M);
}


void App::printDebugHUD() {
    std::cout << std::boolalpha << "State:\n";
    std::cout << "\tflags: done: " << std::setw(5) << state->done <<
        " show_fps: " << std::setw(5) << state->show_fps <<
        " show_map: " << std::setw(5) << state->show_map << '\n';
    std::cout << "\tplayer_pos: " << state->player_pos << " player_dir: " << state->player_dir <<
        " view_plane: " << state->view_plane << '\n';
    std::cout << "Terminal window size:\n\t" << tty_window_h << " rows " << tty_window_w << " columns\n";
    std::cout << "Process Time FPS:\n\t" << (1 / pt_fps_calc.frame_duration_mvg_avg) << '\n';
    std::cout << "Real Time FPS:\n\t" << (1 / rt_fps_calc.frame_duration_mvg_avg.count()) << '\n';

    for (const auto& pair : state->key_handler.key_states) {
        auto key { pair.second };
        std::cout /*<< std::setw(6)*/ << pair.second.repr << ": " << std::noboolalpha << key.isPressed() << ' ';
    }
    std::cout << '\n' << XtermCodes::CursorUp(10);
}

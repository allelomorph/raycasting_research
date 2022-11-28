#include "App.hh"
#include "Matrix.hh"         // Matrix2d Vector2d
#include "safeCExec.hh"      // C_*
#include "XtermCodes.hh"     // CursorUp

#include <time.h>            // clock clock_t CLOCKS_PER_SEC
#include <csignal>           // sigaction SIG* sig_atomic_t
#include <linux/input.h>     // KEY_*
#include <termios.h>         // winsize

#include <sys/types.h>       // pid_t
#include <unistd.h>          // getpid

#include <cmath>             // sin cos
#include <cstdio>            // sprintf
#include <cstring>           // strlen memset

#include <iostream>
#include <iomanip>           // setw
#include <string>


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

    state->base_movement_rate = 5.0;
    state->turn_rate = 0.6;           // 3.0

    pt_fps_calc.initialize();
    rt_fps_calc.initialize();

    state->key_handler.initialize(exec_filename);

    // get terminal window size in chars
    // TBD: add note about finding this ioctl in tput or stty source
    struct winsize winsz;
    safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
              0/*STDIN_FILENO*/, TIOCGWINSZ, &winsz);
    // TBD: will need to redraw frame if HUD or map drawing reuses chars
    // TBD: shade only for testing map and hud
    screen_buffer.resizeToDims(winsz.ws_col, winsz.ws_row);


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
    // TBD: could just be same as drawScreen on empty screen_buffer
    for (uint16_t y {0}; y < screen_buffer.h; ++y) {
        std::cout << std::string(screen_buffer.w, ' ') << '\n';
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
            // TBD: shade only for testing map and hud
            screen_buffer.resizeToDims(winsz.ws_col, winsz.ws_row);
            // TBD: will need to redraw frame if HUD or map drawing reuses chars
            sigwinch_received = 0;
        }

        // printDebugHUD();
        renderView();
        // renderMap();
        renderHUD();

        // TBD: debug errors on terminal window size changes
        if (sigwinch_received)
            continue;

        drawScreen();
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
    double move_speed { pt_fps_calc.frame_duration_mvg_avg * state->base_movement_rate };
    double rot_speed  { pt_fps_calc.frame_duration_mvg_avg *
                        state->base_movement_rate * state->turn_rate };

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

    // f key: toggle FPS overlay
    state->show_fps = state->key_handler.isPressed(KEY_F);

    // m key: toggle map overlay
    state->show_map = state->key_handler.isPressed(KEY_M);
}

void App::renderView() {
    // TBD: dummy fill of buffer while testing renderMap and renderHUD
    static uint16_t chase_x { 2 };
    if (chase_x >= screen_buffer.w)
        chase_x = 2;
    for (auto& c : screen_buffer)
        c = ' ';
    static constexpr char row_labels[] { "0123456789ABCDEFGHIKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz" };
    for (uint16_t y {0}; y < screen_buffer.h && !sigwinch_received; ++y) {
        screen_buffer.charAtCoords(0, y) = row_labels[y];
        screen_buffer.charAtCoords(1, y) = ' ';
        screen_buffer.charAtCoords(chase_x, y) = '|';
    }
    ++chase_x;
}

void App::renderMap() {
}

// TBD: not yet ready to use C++20 std::format, so reverting to C-like printf
//   syntax for constant decimal precsion
void App::renderHUD() {
    //    PTFPS: 4---.2-f  RTFPS: 4---.2-f
    //
    //     done: 0 show_fps: 0 show_map: 0
    //    player_pos: {3--.3--f, 3--.3--f}
    //    player_dir: {3--.3--f, 3--.3--f}
    //    view_plane: {3--.3--f, 3--.3--f}
    //             window size: 67 h 237 w
    //                    user input keys:
    //      down: 0 right: 0 up: 0 left: 0
    //                           a: 0 d: 0
    //                      p: 0 m: 0 f: 0
    // <Rctrl>: 0 <Lctrl>: 0 c: 0 <esc>: 0
    //

    // using sprintf directly on screen_buffer would overwrite null byte after each string
    char hud_line[50] { '\0' };
    auto hud_line_sz { std::strlen(hud_line) };
    uint16_t replace_x;
    if (state->show_fps || state->debug_mode) {
        // dddd.dd format with no negative values
        std::sprintf(hud_line, " PTFPS: %7.2f  RTFPS: %7.2f",
                     (1 / pt_fps_calc.frame_duration_mvg_avg),
                     (1 / rt_fps_calc.frame_duration_mvg_avg.count()) );
        hud_line_sz = std::strlen(hud_line);
        // right justified on first line of screen
        replace_x = screen_buffer.w - hud_line_sz;
        screen_buffer.replaceAtCoords(replace_x, 0,
                                      hud_line_sz, hud_line);
        // blank bottom border line of same length
        std::memset(hud_line, ' ', hud_line_sz);
        hud_line[hud_line_sz] = '\0';
        screen_buffer.replaceAtCoords(replace_x, 1,
                                      hud_line_sz, hud_line);
    }

    if (state->debug_mode && screen_buffer.h >= 13) {
        std::sprintf(hud_line, "     done: %i show_fps: %i show_map: %i",
                     state->done, state->show_fps, state->show_map);
        hud_line_sz = std::strlen(hud_line);
        // also right justified (all lines should be left padded to equal length)
        replace_x = screen_buffer.w - hud_line_sz;
        // -ddd.ddd format
        screen_buffer.replaceAtCoords(replace_x, 2,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    player_pos: {%8.3f, %8.3f}",
                     state->player_pos(0), state->player_pos(1));
        screen_buffer.replaceAtCoords(replace_x, 3,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    player_dir: {%8.3f, %8.3f}",
                     state->player_dir(0), state->player_dir(1));
        screen_buffer.replaceAtCoords(replace_x, 4,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    view_plane: {%8.3f, %8.3f}",
                     state->view_plane(0), state->view_plane(1));
        screen_buffer.replaceAtCoords(replace_x, 5,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "           window size: %3u h %4u w",
                     screen_buffer.h, screen_buffer.w);
        screen_buffer.replaceAtCoords(replace_x, 6,
                                      hud_line_sz, hud_line);

        auto& key_handler { state->key_handler };
        std::sprintf(hud_line, "                    user input keys:");
        screen_buffer.replaceAtCoords(replace_x, 7,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "      down: %i right: %i up: %i left: %i",
                     key_handler.isPressed(KEY_DOWN), key_handler.isPressed(KEY_RIGHT),
                     key_handler.isPressed(KEY_UP), key_handler.isPressed(KEY_LEFT));
        screen_buffer.replaceAtCoords(replace_x, 8,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "                           a: %i d: %i",
                     key_handler.isPressed(KEY_A), key_handler.isPressed(KEY_D));
        screen_buffer.replaceAtCoords(replace_x, 9,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "                      p: %i m: %i f: %i",
                     key_handler.isPressed(KEY_P), key_handler.isPressed(KEY_M),
                     key_handler.isPressed(KEY_F));
        screen_buffer.replaceAtCoords(replace_x, 10,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, " <Lctrl>: %i <Rctrl>: %i c: %i <esc>: %i",
                     key_handler.isPressed(KEY_LEFTCTRL),
                     key_handler.isPressed(KEY_RIGHTCTRL),
                     key_handler.isPressed(KEY_C), key_handler.isPressed(KEY_ESC));
        screen_buffer.replaceAtCoords(replace_x, 11,
                                      hud_line_sz, hud_line);
        // final blank border line
        std::memset(hud_line, ' ', hud_line_sz);
        hud_line[hud_line_sz] = '\0';
        screen_buffer.replaceAtCoords(replace_x, 12,
                                      hud_line_sz, hud_line);
    }
}


// TBD: debug first line remaining after close
void App::drawScreen() {
    for (uint16_t y { 0 }; y < screen_buffer.h; ++y) {
        std::cout << screen_buffer.row(y);
        if (y != screen_buffer.h - 1)
            std::cout << '\n';
    }
    std::cout << XtermCodes::CursorHome()/*XtermCodes::CursorUp(screen_buffer.h - 1)*/;
}

/*
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
        std::cout *<< std::setw(6)* << pair.second.repr << ": " << std::noboolalpha << key.isPressed() << ' ';
    }
    std::cout << '\n' << XtermCodes::CursorUp(10);
}
*/

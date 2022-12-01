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

#include <cmath>             // sin cos M_PI
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
    //   2 * atan(0.66 (magnitude of view_plane) / 1.0 (magnitude of player_dir)), or 66°.
    // Both magnitudes could change provided they stay in proportion, but here
    //   for convenience we make the direction a unit vector.
    state->player_dir << 0, 1;
    state->view_plane << 2.0/3, 0;

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

    state->map_h = screen_buffer.h * state->map_proportion;
    if (state->map_h % 2 == 0)
        ++(state->map_h);
    state->map_w = (state->map_h * 2) + 1;

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
            screen_buffer.resizeToDims(winsz.ws_col, winsz.ws_row);

            state->map_h = screen_buffer.h * state->map_proportion;
            if (state->map_h % 2 == 0)
                ++(state->map_h);
            state->map_w = (state->map_h * 2) + 1;

            sigwinch_received = 0;
        }

        // printDebugHUD();
        renderView();
        renderMap();
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
// TBD: make member function in Matrix.hh?
// rotate Vector counterclockwise around origin
static Vector2d rotateVector2d(const Vector2d& vec, const double radians) {
/*
    Matrix2d rotate;
    rotate <<
        std::cos(rot_speed), -std::sin(rot_speed),
        std::sin(rot_speed), std::cos(rot_speed);
    return (rotate * vector);
*/
    // TBD: currently using manual transposition of vector rotation formula;
    //   debug the above rotation matrix population order
    return Vector2d {
        std::cos(radians) * vec(0) - std::sin(radians) * vec(1),
        std::sin(radians) * vec(0) + std::cos(radians) * vec(1)
    };
}

void App::updateData() {
    double move_speed { pt_fps_calc.frame_duration_mvg_avg * state->base_movement_rate };
    double rot_speed  { pt_fps_calc.frame_duration_mvg_avg *
                        state->base_movement_rate * state->turn_rate };

    Layout* layout { state->layout };
    double& player_dir_x { state->player_dir(0) };
    double& player_dir_y { state->player_dir(1) };
    double& player_pos_x { state->player_pos(0) };
    double& player_pos_y { state->player_pos(1) };

    // q or escape keys: quit
    if (state->key_handler.isPressed(KEY_Q) ||
        state->key_handler.isPressed(KEY_ESC)) {
        state->done = true;
        return;
    }

    // left arrow key: rotate left (CCW)
    if (state->key_handler.isPressed(KEY_LEFT)) {
        state->player_dir = rotateVector2d(state->player_dir, rot_speed);
        state->view_plane = rotateVector2d(state->view_plane, rot_speed);
    }

    // right arrow key: roatate right (CW)
    if (state->key_handler.isPressed(KEY_RIGHT)) {
        state->player_dir = rotateVector2d(state->player_dir, -rot_speed);
        state->view_plane = rotateVector2d(state->view_plane, -rot_speed);
    }

    // Movement in the 2d map grid has been set up so +y/+i with map[i][j]/up on a
    //   printed map all represent moving north
    // up arrow key: move forward
    if (state->key_handler.isPressed(KEY_UP)) {
        double start_ppx { player_pos_x };
        if (!layout->coordsInsideWall(player_pos_x + player_dir_x * move_speed, player_pos_y))
            player_pos_x += player_dir_x * move_speed;
        if (!layout->coordsInsideWall(start_ppx, player_pos_y + player_dir_y * move_speed))
            player_pos_y += player_dir_y * move_speed;
    }

    // down arrow key: move backward
    if (state->key_handler.isPressed(KEY_DOWN)) {
        double start_ppx { player_pos_x };
        if (!layout->coordsInsideWall(player_pos_x - player_dir_x * move_speed, player_pos_y))
            player_pos_x -= player_dir_x * move_speed;
        if (!layout->coordsInsideWall(start_ppx, player_pos_y - player_dir_y * move_speed))
            player_pos_y -= player_dir_y * move_speed;
    }

    // a key: move left (strafe)
    if (state->key_handler.isPressed(KEY_A)) {
        double start_ppx { player_pos_x };
        if (!layout->coordsInsideWall(player_pos_x - player_dir_y * move_speed, player_pos_y))
            player_pos_x -= player_dir_y * move_speed;
        if (!layout->coordsInsideWall(start_ppx, player_pos_y + player_dir_x * move_speed))
            player_pos_y += player_dir_x * move_speed;

    }

    // d key: move right (strafe)
    if (state->key_handler.isPressed(KEY_D)) {
        double start_ppx { player_pos_x };
        if (!layout->coordsInsideWall(player_pos_x + player_dir_y * move_speed, player_pos_y))
            player_pos_x += player_dir_y * move_speed;
        if (!layout->coordsInsideWall(start_ppx, player_pos_y - player_dir_x * move_speed))
            player_pos_y -= player_dir_x * move_speed;

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


// https://stackoverflow.com/questions/6247153/angle-from-2d-unit-vector
static constexpr inline double radiansToDegrees(const double radians) {
    return radians * (180 / M_PI);
}

double vectorAngle(const Vector2d& vec) {
    double angle { radiansToDegrees(std::atan(vec(1) / vec(0))) };
    if (vec(0) < 0) // quadrant II or III
         angle = 180 + angle; // subtracts
    else if (vec(1) < 0) // quadrant IV
         angle = 270 + (90 + angle); // subtracts
    return angle;
}

void App::renderMap() {
    // assert(state->map_dims % 2);
    if (!state->show_map || state->map_h < 5 || state->map_w >= screen_buffer.w)
        return;
    std::string line;
    uint16_t display_row { 0 };
    uint16_t bordered_map_w ( state->map_w + 2 );
    // top border
    line.resize(bordered_map_w, ' ');
    screen_buffer.replaceAtCoords(0, display_row, bordered_map_w,
                                  line.c_str());
    ++display_row;
    uint16_t player_x ( state->player_pos(0) );
    uint16_t player_y ( state->player_pos(1) );
    uint16_t map_delta_y ( state->map_h / 2 );
    uint16_t map_delta_x ( state->map_w / 2 );
    auto* layout { state->layout };
    for (int16_t map_y ( player_y + map_delta_y );
         map_y >= player_y - map_delta_y; --map_y, ++display_row) {
        line.clear();
        line.push_back(' ');  // left border
        for (int16_t map_x ( player_x - map_delta_x );
             map_x <= player_x + map_delta_x; ++map_x) {
            if (map_x < 0 || map_y < 0 ||
                map_x >= static_cast<int16_t>(layout->cols) ||
                map_y >= static_cast<int16_t>(layout->rows)) {
                line.push_back('*');
            } else if (layout->map[map_y][map_x] == 0) {
                line.push_back(' ');
            } else {
                line.push_back(layout->map[map_y][map_x] + '0');
            }
        }
        line.push_back(' ');  // right border
        if (map_y == player_y) {
            char player_icon;
            double player_dir_angle { vectorAngle(state->player_dir) };
            if (player_dir_angle < 22.5)
                player_icon = '~';
            else if (player_dir_angle < 67.5)
                player_icon = '/';
            else if (player_dir_angle < 112.5)
                player_icon = '|';
            else if (player_dir_angle < 157.5)
                player_icon = '\\';
            else if (player_dir_angle < 202.5)
                player_icon = '~';
            else if (player_dir_angle < 247.5)
                player_icon = '/';
            else if (player_dir_angle < 292.5)
                player_icon = '|';
            else if (player_dir_angle < 337.5)
                player_icon = '\\';
            else
                player_icon = '~';
            line[bordered_map_w / 2] = player_icon;
        }
        screen_buffer.replaceAtCoords(0, display_row, bordered_map_w,
                                      line.c_str());
    }
    // bottom border
    line.clear();
    line.resize(bordered_map_w, ' ');
    screen_buffer.replaceAtCoords(0, display_row, bordered_map_w,
                                  line.c_str());
}

// TBD: not yet ready to use C++20 std::format, so reverting to C-like printf
//   syntax for constant decimal precsion
void App::renderHUD() {
    //      PTFPS: 4---.2-  RTFPS: 4---.2-
    //
    //     done: 0 show_fps: 0 show_map: 0
    //      player_pos: {3--.3--, 3--.3--}
    //      player_dir: {3--.3--, 3--.3--}
    //      view_plane: {3--.3--, 3--.3--}
    //           window size: 3-- h 4--- w
    //                    user input keys:
    //      down: 0 right: 0 up: 0 left: 0
    //                           a: 0 d: 0
    //                      p: 0 m: 0 f: 0
    //       Rctrl: 0 Lctrl: 0 c: 0 esc: 0
    //       player_dir magnitude: 3--.3--
    //       view_plane magnitude: 3--.3--
    //    view_plane : player_dir: 3--.3--
    //    player dir angle from +x: 3--.2-
    //

    // using sprintf directly on screen_buffer would also write '\0' after each
    //   line, so instead we use std::string::replace
    char hud_line[50] { '\0' };
    auto hud_line_sz { std::strlen(hud_line) };
    uint16_t replace_col;
    if (state->show_fps || state->debug_mode) {
        // dddd.dd format with no negative values
        std::sprintf(hud_line, " PTFPS: %7.2f  RTFPS: %7.2f",
                     (1 / pt_fps_calc.frame_duration_mvg_avg),
                     (1 / rt_fps_calc.frame_duration_mvg_avg.count()) );
        hud_line_sz = std::strlen(hud_line);
        // right justified on first line of screen
        replace_col = screen_buffer.w - hud_line_sz;
        screen_buffer.replaceAtCoords(replace_col, 0,
                                      hud_line_sz, hud_line);
        // blank bottom border line of same length
        std::memset(hud_line, ' ', hud_line_sz);
        hud_line[hud_line_sz] = '\0';
        screen_buffer.replaceAtCoords(replace_col, 1,
                                      hud_line_sz, hud_line);
    }

    if (state->debug_mode && screen_buffer.h >= 13) {
        std::sprintf(hud_line, "     done: %i show_fps: %i show_map: %i",
                     state->done, state->show_fps, state->show_map);
        hud_line_sz = std::strlen(hud_line);
        // also right justified (all lines should be left padded to equal length)
        replace_col = screen_buffer.w - hud_line_sz;
        // -ddd.ddd format
        screen_buffer.replaceAtCoords(replace_col, 2,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    player_pos: {%8.3f, %8.3f}",
                     state->player_pos(0), state->player_pos(1));
        screen_buffer.replaceAtCoords(replace_col, 3,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    player_dir: {%8.3f, %8.3f}",
                     state->player_dir(0), state->player_dir(1));
        screen_buffer.replaceAtCoords(replace_col, 4,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    view_plane: {%8.3f, %8.3f}",
                     state->view_plane(0), state->view_plane(1));
        screen_buffer.replaceAtCoords(replace_col, 5,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "           window size: %3u h %4u w",
                     screen_buffer.h, screen_buffer.w);
        screen_buffer.replaceAtCoords(replace_col, 6,
                                      hud_line_sz, hud_line);

        auto& key_handler { state->key_handler };
        std::sprintf(hud_line, "                    user input keys:");
        screen_buffer.replaceAtCoords(replace_col, 7,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "      down: %i right: %i up: %i left: %i",
                     key_handler.isPressed(KEY_DOWN), key_handler.isPressed(KEY_RIGHT),
                     key_handler.isPressed(KEY_UP), key_handler.isPressed(KEY_LEFT));
        screen_buffer.replaceAtCoords(replace_col, 8,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "                           a: %i d: %i",
                     key_handler.isPressed(KEY_A), key_handler.isPressed(KEY_D));
        screen_buffer.replaceAtCoords(replace_col, 9,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "                      p: %i m: %i f: %i",
                     key_handler.isPressed(KEY_P), key_handler.isPressed(KEY_M),
                     key_handler.isPressed(KEY_F));
        screen_buffer.replaceAtCoords(replace_col, 10,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "       Lctrl: %i Rctrl: %i c: %i esc: %i",
                     key_handler.isPressed(KEY_LEFTCTRL),
                     key_handler.isPressed(KEY_RIGHTCTRL),
                     key_handler.isPressed(KEY_C), key_handler.isPressed(KEY_ESC));
        screen_buffer.replaceAtCoords(replace_col, 11,
                                      hud_line_sz, hud_line);
        double player_dir_len { std::sqrt(
                state->player_dir(0) * state->player_dir(0) +
                state->player_dir(1) * state->player_dir(1)) };
        std::sprintf(hud_line, "      player_dir magnitude: %8.3f",
                     player_dir_len);
        screen_buffer.replaceAtCoords(replace_col, 12,
                                      hud_line_sz, hud_line);
        double view_plane_len { std::sqrt(
                state->view_plane(0) * state->view_plane(0) +
                state->view_plane(1) * state->view_plane(1)) };
        std::sprintf(hud_line, "      view_plane magnitude: %8.3f",
                     view_plane_len);
        screen_buffer.replaceAtCoords(replace_col, 13,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "   view_plane : player_dir: %8.3f",
                     view_plane_len / player_dir_len);
        screen_buffer.replaceAtCoords(replace_col, 14,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "   player dir angle from +x: %7.2f",
                     vectorAngle(state->player_dir) );
        screen_buffer.replaceAtCoords(replace_col, 15,
                                      hud_line_sz, hud_line);

        // final blank border line
        std::memset(hud_line, ' ', hud_line_sz);
        hud_line[hud_line_sz] = '\0';
        screen_buffer.replaceAtCoords(replace_col, 16,
                                      hud_line_sz, hud_line);
    }
}


// TBD: debug first line remaining after close
void App::drawScreen() {
    // subtraction implicitly converts to int
    uint16_t last_row ( screen_buffer.h - 1 );
    for (uint16_t row { 0 }; row < last_row; ++row)
        std::cout << screen_buffer.row(row) << '\n';
    // TBD: last line could instead be used for notifications and collecting
    //   user text input, eg loading a new map file
    // newline in last row would scroll screen up
    std::cout << screen_buffer.row(last_row);

    std::cout << XtermCodes::CursorHome();
}

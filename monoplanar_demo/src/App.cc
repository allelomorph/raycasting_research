#include "App.hh"            // FovRay WallOrientation TBD: move both to RaycastEngine
#include "Matrix.hh"         // Matrix2d Vector2d
#include "safeCExec.hh"      // C_*
#include "Xterm.hh"          // CtrlSeqs
#include "LinuxKbdInputMgr.hh"
//#include "SdlKbdInputMgr.hh"

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


App::App(const char* efn, const std::string& mfn) :
    exec_filename(efn), map_filename(mfn) {}

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

    // TBD: add test for SDL or TTY mode
    kbd_input_mgr = std::unique_ptr<LinuxKbdInputMgr>(
        new LinuxKbdInputMgr(exec_filename));

    // parse map file to get maze and starting actor positions
    layout.loadMapFile(map_filename, raycast_engine.player_pos);

    // TBD: move to DisplayMgr
    // get terminal window size in chars
    // TBD: add note about finding this ioctl in tput or stty source
    struct winsize winsz;
    safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
              0/*STDIN_FILENO*/, TIOCGWINSZ, &winsz);
    // TBD: will need to redraw frame if HUD or map drawing reuses chars
    // TBD: shade only for testing map and hud
    screen_buffer.resizeToDims(winsz.ws_col, winsz.ws_row);

    raycast_engine.updateScreenSize(screen_buffer.w);

    // TBD: move to DisplayMgr
    settings.map_h = screen_buffer.h * settings.map_proportion;
    if (settings.map_h % 2 == 0)
        ++(settings.map_h);
    settings.map_w = (settings.map_h * 2) + 1;

    // TBD: only do this in tty mode
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
    // TBD: move to DisplayMgr
    for (uint16_t y {0}; y < screen_buffer.h; ++y) {
        std::cout << std::string(screen_buffer.w, ' ') << '\n';
    }
    std::cout << Xterm::CtrlSeqs::CursorHome() << Xterm::CtrlSeqs::EraseLinesBelow() <<
        Xterm::CtrlSeqs::HideCursor();
}

void App::run() {
    initialize();

    // TBD: better consolidate these two tests
    while(!sigint_sigterm_received && !stop) {
        pt_fps_calc.calculate();
        rt_fps_calc.calculate();

        getEvents();
        updateData();

        if (sigwinch_received) {
            // terminal window size changes require rehiding the cursor
            std::cout << Xterm::CtrlSeqs::CursorHome() << Xterm::CtrlSeqs::EraseLinesBelow() <<
                Xterm::CtrlSeqs::HideCursor();
            struct winsize winsz;
            safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
                      0/*STDIN_FILENO*/, TIOCGWINSZ, &winsz);
            screen_buffer.resizeToDims(winsz.ws_col, winsz.ws_row);

            raycast_engine.updateScreenSize(screen_buffer.w);

            settings.map_h = screen_buffer.h * settings.map_proportion;
            if (settings.map_h % 2 == 0)
                ++(settings.map_h);
            settings.map_w = (settings.map_h * 2) + 1;

            sigwinch_received = 0;
        }

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
        Xterm::CtrlSeqs::CursorHome() << Xterm::CtrlSeqs::EraseLinesBelow() <<
        Xterm::CtrlSeqs::ShowCursor();
}

void App::getEvents() {
    kbd_input_mgr->consumeKeyEvents();
}

// TBD: make member function in Matrix, or RaycastEngine?
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
    // ctrl+c: simulate SIGINT (quit)
    if ((kbd_input_mgr->isPressed(KEY_LEFTCTRL) ||
         kbd_input_mgr->isPressed(KEY_RIGHTCTRL)) &&
        kbd_input_mgr->isPressed(KEY_C)) {
        stop = true;
        return;
    }

    // q or escape keys: quit
    if (kbd_input_mgr->isPressed(KEY_Q) ||
        kbd_input_mgr->isPressed(KEY_ESC)) {
        stop = true;
        return;
    }

    double move_speed { pt_fps_calc.frame_duration_mvg_avg * settings.base_movement_rate };
    double rot_speed  { pt_fps_calc.frame_duration_mvg_avg *
                        settings.base_movement_rate * settings.turn_rate };
    double& player_dir_x { raycast_engine.player_dir(0) };
    double& player_dir_y { raycast_engine.player_dir(1) };
    double& player_pos_x { raycast_engine.player_pos(0) };
    double& player_pos_y { raycast_engine.player_pos(1) };

    // Movement in the 2d map grid has been set up so +y/+i with map[i][j]/up on a
    //   printed map all represent moving north

    // shift key: run
    if (kbd_input_mgr->isPressed(KEY_LEFTSHIFT) ||
        kbd_input_mgr->isPressed(KEY_RIGHTSHIFT)) {
        move_speed *= 2;
        rot_speed *= 2;
    }

    // up arrow key: move forward
    if (kbd_input_mgr->isPressed(KEY_UP)) {
        double start_ppx { player_pos_x };
        if (!layout.tileIsWall(player_pos_x + player_dir_x * move_speed, player_pos_y))
            player_pos_x += player_dir_x * move_speed;
        if (!layout.tileIsWall(start_ppx, player_pos_y + player_dir_y * move_speed))
            player_pos_y += player_dir_y * move_speed;
    }

    // down arrow key: move backward
    if (kbd_input_mgr->isPressed(KEY_DOWN)) {
        double start_ppx { player_pos_x };
        if (!layout.tileIsWall(player_pos_x - player_dir_x * move_speed, player_pos_y))
            player_pos_x -= player_dir_x * move_speed;
        if (!layout.tileIsWall(start_ppx, player_pos_y - player_dir_y * move_speed))
            player_pos_y -= player_dir_y * move_speed;
    }

    if (kbd_input_mgr->isPressed(KEY_LEFT)) {
        if (kbd_input_mgr->isPressed(KEY_LEFTALT) ||
            kbd_input_mgr->isPressed(KEY_RIGHTALT)) {
            // alt + left arrow key: move left (strafe)
            double start_ppx { player_pos_x };
            if (!layout.tileIsWall(player_pos_x - player_dir_y * move_speed, player_pos_y))
                player_pos_x -= player_dir_y * move_speed;
            if (!layout.tileIsWall(start_ppx, player_pos_y + player_dir_x * move_speed))
                player_pos_y += player_dir_x * move_speed;
        } else {
            // left arrow key: rotate left (CCW)
            raycast_engine.player_dir = rotateVector2d(raycast_engine.player_dir, rot_speed);
            raycast_engine.view_plane = rotateVector2d(raycast_engine.view_plane, rot_speed);
        }
    }

    if (kbd_input_mgr->isPressed(KEY_RIGHT)) {
        if (kbd_input_mgr->isPressed(KEY_LEFTALT) ||
            kbd_input_mgr->isPressed(KEY_RIGHTALT)) {
            // alt + right arrow key: move right (strafe)
            double start_ppx { player_pos_x };
            if (!layout.tileIsWall(player_pos_x + player_dir_y * move_speed, player_pos_y))
                player_pos_x += player_dir_y * move_speed;
            if (!layout.tileIsWall(start_ppx, player_pos_y - player_dir_x * move_speed))
                player_pos_y -= player_dir_x * move_speed;
        } else {
            // right arrow key: roatate right (CW)
            raycast_engine.player_dir = rotateVector2d(raycast_engine.player_dir, -rot_speed);
            raycast_engine.view_plane = rotateVector2d(raycast_engine.view_plane, -rot_speed);
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


static void renderPixelColumn(const uint16_t screen_x,
                              ASCIIScreenBuffer& screen_buffer, const FovRay& ray) {
    // The core illusion of raycasting comes from rendering walls in vertical
    //   strips, one per each ray cast in the FOV, with each strip being longer
    //   as the ray is shorter/wall is closer, forcing perspective.

    // calculate height of vertical strip of wall to draw on screen
    uint16_t line_h ( screen_buffer.h / ray.wall_dist );

    // calculate lowest and highest pixel in strip, trimmed to screen borders
    uint16_t ceiling_screen_y ( (line_h >= screen_buffer.h) ?
                          0 : screen_buffer.h / 2 - line_h / 2 );
    uint16_t floor_screen_y ( (line_h >= screen_buffer.h) ?
                          screen_buffer.h - 1 : screen_buffer.h / 2 + line_h / 2 );

    uint16_t screen_y;
    // draw ceiling
    for (screen_y = 0; screen_y < ceiling_screen_y; ++screen_y)
        screen_buffer.charAtCoords(screen_x, screen_y) = ' ';
    // draw wall, shading NS walls darker to differentiate
    for (; screen_y <= floor_screen_y; ++screen_y) {
        screen_buffer.charAtCoords(screen_x, screen_y) =
            (ray.type_wall_hit == WallOrientation::EW) ? '@' : '|';
    }
    // draw floor
    for (; screen_y < screen_buffer.h; ++screen_y)
        screen_buffer.charAtCoords(screen_x, screen_y) = ' ';
}

void App::renderView() {
    // raycasting loop: one column of pixels (screen coordinates) per pass
    for (uint16_t screen_x { 0 }; screen_x < screen_buffer.w; ++screen_x) {
        FovRay ray { raycast_engine.castRay(screen_x, layout, settings) };
        renderPixelColumn(screen_x, screen_buffer, ray);
    }
}

// https://stackoverflow.com/questions/6247153/angle-from-2d-unit-vector
constexpr inline double radiansToDegrees(const double radians) {
    return radians * (180 / M_PI);
}

double vectorAngle(const Vector2d& vec) {
    double angle { radiansToDegrees(std::atan(vec(1) / vec(0))) };
    if (vec(0) < 0)  // quadrant II or III
         angle = 180 + angle;  // subtracts
    else if (vec(1) < 0)  // quadrant IV
         angle = 270 + (90 + angle);  // subtracts
    return angle;
}

void App::renderMap() {
    // assert(settings.map_dims % 2);
    if (!settings.show_map || settings.map_h < 5 || settings.map_w >= screen_buffer.w)
        return;
    std::string line;
    uint16_t display_row { 0 };
    uint16_t bordered_map_w ( settings.map_w + 2 );
    // top border
    line.resize(bordered_map_w, ' ');
    screen_buffer.replaceAtCoords(0, display_row, bordered_map_w,
                                  line.c_str());
    ++display_row;
    uint16_t player_x ( raycast_engine.player_pos(0) );
    uint16_t player_y ( raycast_engine.player_pos(1) );
    uint16_t map_delta_y ( settings.map_h / 2 );
    uint16_t map_delta_x ( settings.map_w / 2 );
    for (int16_t map_y ( player_y + map_delta_y );
         map_y >= player_y - map_delta_y; --map_y, ++display_row) {
        line.clear();
        line.push_back(' ');  // left border
        for (int16_t map_x ( player_x - map_delta_x );
             map_x <= player_x + map_delta_x; ++map_x) {
            if (map_x < 0 || map_y < 0 ||
                map_x >= static_cast<int16_t>(layout.w) ||
                map_y >= static_cast<int16_t>(layout.h) ||
                layout.tileData(map_x, map_y) == 0) {
                line.push_back(' ');
            } else {
                line.push_back(layout.tileData(map_x, map_y) + '0');
            }
        }
        line.push_back(' ');  // right border
        if (map_y == player_y) {
            char player_icon;
            double player_dir_angle { vectorAngle(raycast_engine.player_dir) };
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
    if (settings.show_fps || settings.debug_mode) {
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

    if (settings.debug_mode && screen_buffer.h >= 13) {
        std::sprintf(hud_line, "     stop: %i show_fps: %i show_map: %i",
                     stop, settings.show_fps, settings.show_map);
        hud_line_sz = std::strlen(hud_line);
        // also right justified (all lines should be left padded to equal length)
        replace_col = screen_buffer.w - hud_line_sz;
        // -ddd.ddd format
        screen_buffer.replaceAtCoords(replace_col, 2,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    player_pos: {%8.3f, %8.3f}",
                     raycast_engine.player_pos(0), raycast_engine.player_pos(1));
        screen_buffer.replaceAtCoords(replace_col, 3,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    player_dir: {%8.3f, %8.3f}",
                     raycast_engine.player_dir(0), raycast_engine.player_dir(1));
        screen_buffer.replaceAtCoords(replace_col, 4,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "    view_plane: {%8.3f, %8.3f}",
                     raycast_engine.view_plane(0), raycast_engine.view_plane(1));
        screen_buffer.replaceAtCoords(replace_col, 5,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "           window size: %3u h %4u w",
                     screen_buffer.h, screen_buffer.w);
        screen_buffer.replaceAtCoords(replace_col, 6,
                                      hud_line_sz, hud_line);

        std::sprintf(hud_line, "                    user input keys:");
        screen_buffer.replaceAtCoords(replace_col, 7,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "      down: %i right: %i up: %i left: %i",
                     kbd_input_mgr->isPressed(KEY_DOWN), kbd_input_mgr->isPressed(KEY_RIGHT),
                     kbd_input_mgr->isPressed(KEY_UP), kbd_input_mgr->isPressed(KEY_LEFT));
        screen_buffer.replaceAtCoords(replace_col, 8,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "                           a: %i d: %i",
                     kbd_input_mgr->isPressed(KEY_A), kbd_input_mgr->isPressed(KEY_D));
        screen_buffer.replaceAtCoords(replace_col, 9,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "                      p: %i m: %i f: %i",
                     kbd_input_mgr->isPressed(KEY_P), kbd_input_mgr->isPressed(KEY_M),
                     kbd_input_mgr->isPressed(KEY_F));
        screen_buffer.replaceAtCoords(replace_col, 10,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "       Lctrl: %i Rctrl: %i c: %i esc: %i",
                     kbd_input_mgr->isPressed(KEY_LEFTCTRL),
                     kbd_input_mgr->isPressed(KEY_RIGHTCTRL),
                     kbd_input_mgr->isPressed(KEY_C), kbd_input_mgr->isPressed(KEY_ESC));
        screen_buffer.replaceAtCoords(replace_col, 11,
                                      hud_line_sz, hud_line);
        double player_dir_len { std::sqrt(
                raycast_engine.player_dir(0) * raycast_engine.player_dir(0) +
                raycast_engine.player_dir(1) * raycast_engine.player_dir(1)) };
        std::sprintf(hud_line, "      player_dir magnitude: %8.3f",
                     player_dir_len);
        screen_buffer.replaceAtCoords(replace_col, 12,
                                      hud_line_sz, hud_line);
        double view_plane_len { std::sqrt(
                raycast_engine.view_plane(0) * raycast_engine.view_plane(0) +
                raycast_engine.view_plane(1) * raycast_engine.view_plane(1)) };
        std::sprintf(hud_line, "      view_plane magnitude: %8.3f",
                     view_plane_len);
        screen_buffer.replaceAtCoords(replace_col, 13,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "   view_plane : player_dir: %8.3f",
                     view_plane_len / player_dir_len);
        screen_buffer.replaceAtCoords(replace_col, 14,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "   player dir angle from +x: %7.2f",
                     vectorAngle(raycast_engine.player_dir) );
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

    std::cout << Xterm::CtrlSeqs::CursorHome();
}

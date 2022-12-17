#include "TtyDisplayMgr.hh"
#include "safeCExec.hh"               // C_*
#include "Xterm.hh"                   // CtrlSeqs::

#include <termios.h>                  // winsize
#include <sys/ioctl.h>
#include <unistd.h>                   // ttyname STDIN_FILENO
#include <linux/input-event-codes.h>  // KEY_*

#include <cassert>
#include <cmath>                      // sin cos M_PI
#include <cstdio>                     // sprintf
#include <cstring>                    // strlen memset

#include <iostream>
#include <iomanip>                    // setw
#include <string>
#include <sstream>


void TtyDisplayMgr::updateMiniMapSize(const double map_proportion) {
    assert(map_proportion > 0.0);
    minimap_h = screen_buffer.h * map_proportion;
    // dims must be odd to center player icon
    if (minimap_h % 2 == 0)
        ++(minimap_h);
    minimap_w = (minimap_h * 2) + 1;
}

void TtyDisplayMgr::fitToWindow(const double map_proportion) {
    // get terminal window size in chars
    // use of TIOCGWINSZ taken from coreutils stty, see:
    //   - https://github.com/wertarbyte/coreutils/blob/master/src/stty.c#L1311
    struct winsize winsz;
    safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
              STDIN_FILENO, TIOCGWINSZ, &winsz);
    screen_buffer.resize(winsz.ws_col, winsz.ws_row);

    updateMiniMapSize(map_proportion);
}

// TBD remove?
void TtyDisplayMgr::clearDisplay() {
    std::cout << Xterm::CtrlSeqs::CursorHome() <<
        Xterm::CtrlSeqs::EraseLinesBelow();
}

void TtyDisplayMgr::initialize(const double map_proportion) {
    // Use of ttyname taken from coreutils tty, see:
    //  - https://github.com/coreutils/coreutils/blob/master/src/tty.c
    tty_name = safeCExec(ttyname, "ttyname",
                         C_RETURN_TEST(char *, (ret == nullptr)),
                         STDIN_FILENO);

    fitToWindow(map_proportion);

    // force scrollback of all terminal text by drawing an empty frame
    //   (screen_buffer default init is to all black ' ' chars)
    drawScreen();
    clearDisplay();
    std::cout << Xterm::CtrlSeqs::HideCursor();
}

// TBD: make public member? In case of building for optional multithreading,
//   each thread would have to trace a batch of rays, then render their
//   respective pixel columns without stopping to wait for the others,
//   which means each thread would have to call this.
// The core illusion of raycasting comes from rendering walls in vertical
//   strips, one per each ray cast in the FOV, with each strip being longer
//   as the ray is shorter/wall is closer, forcing perspective.
void TtyDisplayMgr::renderPixelColumn(const uint16_t screen_x,
                                      const FovRay& ray) {

    // calculate height of vertical strip of wall to draw on screen
    uint16_t line_h ( screen_buffer.h / ray.wall_dist );

    // calculate lowest and highest pixel in strip, trimmed to screen borders
    uint16_t ceiling_screen_y;
    uint16_t floor_screen_y;
    if (line_h >= screen_buffer.h) {
        ceiling_screen_y = 0;
        floor_screen_y = screen_buffer.h - 1;
    } else {
        ceiling_screen_y = screen_buffer.h / 2 - line_h / 2;
        floor_screen_y = screen_buffer.h / 2 + line_h / 2;
    }

    // TBD: add test for ascii/256/truecolor modes
    uint16_t screen_y;
    // draw ceiling
    for (screen_y = 0; screen_y < ceiling_screen_y; ++screen_y)
        screen_buffer.pixel(screen_x, screen_y).c = ' ';
    // draw wall, shading NS walls darker to differentiate
    for (; screen_y <= floor_screen_y; ++screen_y) {
        screen_buffer.pixel(screen_x, screen_y).c =
            (ray.type_wall_hit == WallOrientation::EW) ? '@' : '|';
    }
    // draw floor
    for (; screen_y < screen_buffer.h; ++screen_y)
        screen_buffer.pixel(screen_x, screen_y).c = ' ';
}

void TtyDisplayMgr::renderView(const std::vector<FovRay>& fov_rays) {
    for (uint16_t screen_x { 0 }; screen_x < screen_buffer.w; ++screen_x)
        renderPixelColumn(screen_x, fov_rays[screen_x]);
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

void TtyDisplayMgr::renderMap(const DdaRaycastEngine& raycast_engine
                              /*const Layout& layout, const Vector2d& player_pos, const Vector2d& player_dir*/) {
    // assert(state->map_dims % 2);
    if (minimap_h < 5 || minimap_w >= screen_buffer.w)
        return;
    std::string line;
    uint16_t display_row_i { 0 };
    uint16_t bordered_map_w ( minimap_w + 2 );
    // top border
    line.resize(bordered_map_w, ' ');
    screen_buffer.pixelCharReplace(0, display_row_i,
                                   line.c_str(), bordered_map_w);
    ++display_row_i;
    const uint16_t player_x ( raycast_engine.player_pos(0) );
    const uint16_t player_y ( raycast_engine.player_pos(1) );
    const uint16_t map_delta_y ( minimap_h / 2 );
    const uint16_t map_delta_x ( minimap_w / 2 );
    const Layout& layout { raycast_engine.layout };
    for (int16_t map_y ( player_y + map_delta_y );
         map_y >= player_y - map_delta_y; --map_y, ++display_row_i) {
        line.clear();
        line.push_back(' ');  // left border
        for (int16_t map_x ( player_x - map_delta_x );
             map_x <= player_x + map_delta_x; ++map_x) {
            if (map_x < 0 || map_y < 0 ||
                map_x >= static_cast<int16_t>(layout.w) ||
                map_y >= static_cast<int16_t>(layout.h) ||
                !raycast_engine.layout.tileIsWall(map_x, map_y)) {
                line.push_back(' ');
            } else {
                line.push_back(raycast_engine.layout.tileData(map_x, map_y) + '0');
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
        screen_buffer.pixelCharReplace(0, display_row_i,
                                       line.c_str(), bordered_map_w);
    }
    // bottom border
    line.clear();
    line.resize(bordered_map_w, ' ');
    screen_buffer.pixelCharReplace(0, display_row_i,
                                   line.c_str(), bordered_map_w);
}

void TtyDisplayMgr::renderHUD(const double pt_frame_duration_mvg_avg,
                              const double rt_frame_duration_mvg_avg,
                              const Settings& settings,
                              const DdaRaycastEngine& raycast_engine,
                              const std::unique_ptr<KbdInputMgr>& kbd_input_mgr) {
    //      PTFPS: 4---.2-  RTFPS: 4---.2-
    //
    //     stop: 0 show_fps: 0 show_map: 0
    //      player_pos: {3--.3--, 3--.3--}
    //      player_dir: {3--.3--, 3--.3--}
    //      view_plane: {3--.3--, 3--.3--}
    //           window size: 3-- h 4--- w
    //                    user input keys:
    //      down: 0 right: 0 up: 0 left: 0
    //    player dir angle from +x: 3--.2-
    //

    // Using sprintf over idiomatic C++ to get exact precision on floating
    //   point values (not using C++20, so std::format is not an option.)
    char hud_line[50] { '\0' };
    std::size_t hud_line_sz;
    uint16_t replace_col_i;
    if (settings.show_fps || settings.debug_mode) {
        // display FPS at top right in dddd.dd format (no negative values
        //   should appear)
        std::sprintf(hud_line, " PTFPS: %7.2f  RTFPS: %7.2f",
                     (1 / pt_frame_duration_mvg_avg),
                     (1 / rt_frame_duration_mvg_avg) );
        hud_line_sz = std::strlen(hud_line);
        replace_col_i = screen_buffer.w - hud_line_sz;
        screen_buffer.pixelCharReplace(replace_col_i, 0, hud_line, hud_line_sz);
        // blank border of same length underneath
        std::memset(hud_line, ' ', hud_line_sz);
        hud_line[hud_line_sz] = '\0';
        screen_buffer.pixelCharReplace(replace_col_i, 1, hud_line, hud_line_sz);
    }

    if (settings.debug_mode && screen_buffer.h >= 11) {
        // TBD: add final list of settings
        std::sprintf(hud_line, "     stop: X show_fps: %i show_map: %i",
                     settings.show_fps, settings.show_map);
        hud_line_sz = std::strlen(hud_line);
        // right justified (all lines should be left padded to equal length)
        replace_col_i = screen_buffer.w - hud_line_sz;

        // -ddd.ddd format
        screen_buffer.pixelCharReplace(replace_col_i, 2, hud_line, hud_line_sz);
        std::sprintf(hud_line, "    player_pos: {%8.3f, %8.3f}",
                     raycast_engine.player_pos(0), raycast_engine.player_pos(1));
        screen_buffer.pixelCharReplace(replace_col_i, 3, hud_line, hud_line_sz);
        std::sprintf(hud_line, "    player_dir: {%8.3f, %8.3f}",
                     raycast_engine.player_dir(0), raycast_engine.player_dir(1));
        screen_buffer.pixelCharReplace(replace_col_i, 4, hud_line, hud_line_sz);
        std::sprintf(hud_line, "    view_plane: {%8.3f, %8.3f}",
                     raycast_engine.view_plane(0), raycast_engine.view_plane(1));
        screen_buffer.pixelCharReplace(replace_col_i, 5, hud_line, hud_line_sz);

        std::sprintf(hud_line, "           window size: %3u h %4u w",
                     screen_buffer.h, screen_buffer.w);
        screen_buffer.pixelCharReplace(replace_col_i, 6, hud_line, hud_line_sz);

        // TBD: add final key layout
        std::sprintf(hud_line, "                    user input keys:");
        screen_buffer.pixelCharReplace(replace_col_i, 7, hud_line, hud_line_sz);
        std::sprintf(hud_line, "      down: %i right: %i up: %i left: %i",
                     kbd_input_mgr->isPressed(KEY_DOWN), kbd_input_mgr->isPressed(KEY_RIGHT),
                     kbd_input_mgr->isPressed(KEY_UP), kbd_input_mgr->isPressed(KEY_LEFT));
        screen_buffer.pixelCharReplace(replace_col_i, 8, hud_line, hud_line_sz);

        std::sprintf(hud_line, "   player dir angle from +x: %7.2f",
                     vectorAngle(raycast_engine.player_dir) );
        screen_buffer.pixelCharReplace(replace_col_i, 9, hud_line, hud_line_sz);

        // final blank border line
        std::memset(hud_line, ' ', hud_line_sz);
        hud_line[hud_line_sz] = '\0';
        screen_buffer.pixelCharReplace(replace_col_i, 10, hud_line, hud_line_sz);
    }
}

void TtyDisplayMgr::drawScreen() {
    assert(screen_buffer.h > 0);
    std::ostringstream oss;
    TtyPixel px;
    // subtraction implicitly converts to int
    uint16_t last_row_i ( screen_buffer.h - 1 );
    for (uint16_t row_i { 0 }; row_i < last_row_i; ++row_i) {
        for (uint16_t col_i { 0 }; col_i < screen_buffer.w; ++col_i) {
            px = screen_buffer.pixel(col_i, row_i);
/*
            if (mode is 256 color) {
                oss << Xterm::CtrlSeqs::CharBgColor(px.code);
            }
            if (mode is true color) {
                oss << Xterm::CtrlSeqs::CharBgColor(px.r, px.g, px.b);
            }
*/
            oss << px.c;
        }
        oss << '\n';
    }
    // TBD: last line could instead be used for notifications and collecting
    //   user text input, eg loading a new map file
    // newline in last row would scroll screen up
    for (uint16_t col_i { 0 }; col_i < screen_buffer.w; ++col_i) {
            px = screen_buffer.pixel(col_i, last_row_i);
/*
            if (mode is 256 color) {
                oss << Xterm::CtrlSeqs::CharBgColor(px.code);
            }
            if (mode is true color) {
                oss << Xterm::CtrlSeqs::CharBgColor(px.r, px.g, px.b);
            }
*/
            oss << px.c;
    }

    std::cout << oss.str();
    std::cout << Xterm::CtrlSeqs::CursorHome();
}

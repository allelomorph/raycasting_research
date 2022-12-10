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


App::App(const char* efn, const std::string& mfn) :
    state(new State<LinuxKbdInputMgr>),
    exec_filename(efn), map_filename(mfn) {
}

App::~App() {
    if (state != nullptr)
        delete state;
}

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
    state->initialize(exec_filename, map_filename);

    pt_fps_calc.initialize();
    rt_fps_calc.initialize();

    // TBD: move to DisplayMgr
    // get terminal window size in chars
    // TBD: add note about finding this ioctl in tput or stty source
    struct winsize winsz;
    safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
              0/*STDIN_FILENO*/, TIOCGWINSZ, &winsz);
    // TBD: will need to redraw frame if HUD or map drawing reuses chars
    // TBD: shade only for testing map and hud
    screen_buffer.resizeToDims(winsz.ws_col, winsz.ws_row);

    // TBD: move to DisplayMgr
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
    // TBD: move to DisplayMgr
    for (uint16_t y {0}; y < screen_buffer.h; ++y) {
        std::cout << std::string(screen_buffer.w, ' ') << '\n';
    }
    std::cout << XtermCodes::CursorHome() << XtermCodes::EraseLinesBelow() <<
        XtermCodes::HideCursor();
}

void App::run() {
    initialize();

    // TBD: better consolidate these two tests
    while(!sigint_sigterm_received && !state->stop) {
        pt_fps_calc.calculate();
        rt_fps_calc.calculate();

        getEvents();
        updateData();

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
    state->kbd_input_mgr.consumeKeyEvents();
}

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
    // ctrl+c: simulate SIGINT (quit)
    if ((state->kbd_input_mgr.isPressed(KEY_LEFTCTRL) ||
         state->kbd_input_mgr.isPressed(KEY_RIGHTCTRL)) &&
        state->kbd_input_mgr.isPressed(KEY_C)) {
        state->stop = true;
        return;
    }

    // q or escape keys: quit
    if (state->kbd_input_mgr.isPressed(KEY_Q) ||
        state->kbd_input_mgr.isPressed(KEY_ESC)) {
        state->stop = true;
        return;
    }

    double move_speed { pt_fps_calc.frame_duration_mvg_avg * state->base_movement_rate };
    double rot_speed  { pt_fps_calc.frame_duration_mvg_avg *
                        state->base_movement_rate * state->turn_rate };
    Layout& layout { state->layout };
    double& player_dir_x { state->player_dir(0) };
    double& player_dir_y { state->player_dir(1) };
    double& player_pos_x { state->player_pos(0) };
    double& player_pos_y { state->player_pos(1) };

    // Movement in the 2d map grid has been set up so +y/+i with map[i][j]/up on a
    //   printed map all represent moving north

    // shift key: run
    if (state->kbd_input_mgr.isPressed(KEY_LEFTSHIFT) ||
        state->kbd_input_mgr.isPressed(KEY_RIGHTSHIFT)) {
        move_speed *= 2;
        rot_speed *= 2;
    }

    // up arrow key: move forward
    if (state->kbd_input_mgr.isPressed(KEY_UP)) {
        double start_ppx { player_pos_x };
        if (!layout.tileIsWall(player_pos_x + player_dir_x * move_speed, player_pos_y))
            player_pos_x += player_dir_x * move_speed;
        if (!layout.tileIsWall(start_ppx, player_pos_y + player_dir_y * move_speed))
            player_pos_y += player_dir_y * move_speed;
    }

    // down arrow key: move backward
    if (state->kbd_input_mgr.isPressed(KEY_DOWN)) {
        double start_ppx { player_pos_x };
        if (!layout.tileIsWall(player_pos_x - player_dir_x * move_speed, player_pos_y))
            player_pos_x -= player_dir_x * move_speed;
        if (!layout.tileIsWall(start_ppx, player_pos_y - player_dir_y * move_speed))
            player_pos_y -= player_dir_y * move_speed;
    }

    if (state->kbd_input_mgr.isPressed(KEY_LEFT)) {
        if (state->kbd_input_mgr.isPressed(KEY_LEFTALT) ||
            state->kbd_input_mgr.isPressed(KEY_RIGHTALT)) {
            // alt + left arrow key: move left (strafe)
            double start_ppx { player_pos_x };
            if (!layout.tileIsWall(player_pos_x - player_dir_y * move_speed, player_pos_y))
                player_pos_x -= player_dir_y * move_speed;
            if (!layout.tileIsWall(start_ppx, player_pos_y + player_dir_x * move_speed))
                player_pos_y += player_dir_x * move_speed;
        } else {
            // left arrow key: rotate left (CCW)
            state->player_dir = rotateVector2d(state->player_dir, rot_speed);
            state->view_plane = rotateVector2d(state->view_plane, rot_speed);
        }
    }

    if (state->kbd_input_mgr.isPressed(KEY_RIGHT)) {
        if (state->kbd_input_mgr.isPressed(KEY_LEFTALT) ||
            state->kbd_input_mgr.isPressed(KEY_RIGHTALT)) {
            // alt + right arrow key: move right (strafe)
            double start_ppx { player_pos_x };
            if (!layout.tileIsWall(player_pos_x + player_dir_y * move_speed, player_pos_y))
                player_pos_x += player_dir_y * move_speed;
            if (!layout.tileIsWall(start_ppx, player_pos_y - player_dir_x * move_speed))
                player_pos_y -= player_dir_x * move_speed;
        } else {
            // right arrow key: roatate right (CW)
            state->player_dir = rotateVector2d(state->player_dir, -rot_speed);
            state->view_plane = rotateVector2d(state->view_plane, -rot_speed);
        }
    }

    // F1 key: toggle FPS overlay
    if (state->kbd_input_mgr.keyDownThisFrame(KEY_F1))
        state->show_fps = !state->show_fps;

    // F2 key: toggle map overlay
    if (state->kbd_input_mgr.keyDownThisFrame(KEY_F2))
        state->show_map = !state->show_map;

    // F3 key: toggle debug mode
    if (state->kbd_input_mgr.keyDownThisFrame(KEY_F3))
        state->debug_mode = !state->debug_mode;

    // F4 key: toggle fisheye camera mode
    if (state->kbd_input_mgr.keyDownThisFrame(KEY_F4))
        state->fisheye = !state->fisheye;

    state->kbd_input_mgr.decayToAutorepeat();
}

enum class WallOrientation { EW, NS };

struct FovRay {
    Vector2d        dir;              // ray direction
    double          wall_dist;        // distance to first wall collision
    WallOrientation type_wall_hit;    // NS or EW alignment of wall hit
};

/*
 * calculate ray position and direction
 */
// TBD: make compatible with templated State
static FovRay castRay(const uint16_t screen_x, const uint16_t screen_w,
                      State<LinuxKbdInputMgr>* state) {

    // TBD: move camera_x calc outside function, to renderView loop?
    // x coordinate in the camera plane represented by the current
    //   screen x coordinate, calculated so that the left edge of the
    //   camera plane is -1.0, center 0.0, and right edge is 1.0
    double camera_x { 2 * screen_x / double(screen_w) - 1 };

    FovRay ray;

    // ray origin is player_pos
    // multiply camera_plane vector by scalar x, then add to direction vector
    //   to get ray direction
    // TBD: const palyer_dir or view_plane as this discards qualifiers
    ray.dir = state->player_dir + (state->view_plane * camera_x);

    // current map grid coordinates of ray
    uint16_t map_x ( state->player_pos(0) );
    uint16_t map_y ( state->player_pos(1) );

    // Initially the distances from the ray origin (player position) to its
    //   first intersections with a map unit grid vertical and horizonal,
    //   respectively. These map grid lines, or integer values of x and y,
    //   serve to represent wall boundaries when they border a grid square
    //   designated as a wall.
    double dist_next_unit_x;
    double dist_next_unit_y;

    // distances the ray has to travel to go from one unit grid vertical
    //   to the next, or one horizontal to the next, respectively
    // IEEE 754 floating point values in C++ protect against division by 0
    double dist_per_unit_x { std::abs(1 / ray.dir(0)) };
    double dist_per_unit_y { std::abs(1 / ray.dir(1)) };

    // DDA algorithm will always jump exactly one map grid square each
    //   loop, either in the x or y. These vars record those increments,
    //   either -1 or +1
    int8_t map_step_x;
    int8_t map_step_y;

    // setup map grid step and initial ray distance to next grid unit values
    if (ray.dir(0) < 0) {
        map_step_x = -1;
        dist_next_unit_x = (state->player_pos(0) - map_x) * dist_per_unit_x;
    } else {
        map_step_x = 1;
        dist_next_unit_x = (map_x + 1.0 - state->player_pos(0)) * dist_per_unit_x;
    }
    if (ray.dir(1) < 0) {
        map_step_y = -1;
        dist_next_unit_y = (state->player_pos(1) - map_y) * dist_per_unit_y;
    } else {
        map_step_y = 1;
        dist_next_unit_y = (map_y + 1.0 - state->player_pos(1)) * dist_per_unit_y;
    }

    // perform DDA algo, or the incremental casting of the ray
    // moves to a new map unit square every loop, as directed by map_step values
    // TBD: does orientation need to be set every loop?
    for (const Layout& layout { state->layout }; !layout.tileIsWall(map_x, map_y); ) {
        if (dist_next_unit_x < dist_next_unit_y) {
            dist_next_unit_x += dist_per_unit_x;
            map_x += map_step_x;
            ray.type_wall_hit = WallOrientation::EW;
        } else {
            dist_next_unit_y += dist_per_unit_y;
            map_y += map_step_y;
            ray.type_wall_hit = WallOrientation::NS;
        }
        // TBD: what about OneLoneCoder's max cast distance?
    }

    // Calculate distance to wall hit from camera plane, moving
    //   perpendicular to the camera plane. If the actual length of the
    //   ray cast from player to wall were used, the result would be
    //   a fisheye effect. For example, if the player were squarely
    //   facing a wall (wall parallel to camera plane,) in the resulting
    //   render the wall should appear of even height. If using the
    //   length of the rays as they hit the wall, the wall would instead
    //   appear to taper to the left and right ends of the display.
    //
    // One way to calculate the perpendicular camera plane distance would be
    //   to use the formula for shortest distance from a point to a line,
    //   where the point is where the wall was hit, and the line is the
    //   camera plane.
    // TBD: evaluate and corroborate this last paragraph
    // However, it can be done more simply: due to how dist_per_unit_? and
    //   dist_next_unit_? were scaled by a factor of |ray.dir| above, the
    //   length of dist_next_unit_? already almost equals the perpendicular
    //   camera plane distance. We just need to subtract dist_per_unit_? once,
    //   going one step back in the casting, as in the DDA loop above we went
    //   one step further to end up inside the wall.

    // TBD: double check these
    if (state->fisheye) {  // Euclidean ray distance from player_pos
        ray.wall_dist = (ray.type_wall_hit == WallOrientation::EW) ?
            dist_next_unit_x : dist_next_unit_y;
    } else {                // perpendicular distance from camera plane
        ray.wall_dist = (ray.type_wall_hit == WallOrientation::EW) ?
            dist_next_unit_x - dist_per_unit_x :
            dist_next_unit_y - dist_per_unit_y;
    }
    return ray;
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

/*
//dummy fill of buffer while testing renderMap and renderHUD
void App:renderTestChase() {
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
*/

void App::renderView() {
    // raycasting loop: one column of pixels (screen coordinates) per pass
    for (uint16_t screen_x { 0 }; screen_x < screen_buffer.w; ++screen_x) {
        FovRay ray { castRay(screen_x, screen_buffer.w, state) };
        renderPixelColumn(screen_x, screen_buffer, ray);
    }
}


// https://stackoverflow.com/questions/6247153/angle-from-2d-unit-vector
constexpr inline double radiansToDegrees(const double radians) {
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
    Layout& layout { state->layout };
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
        std::sprintf(hud_line, "     stop: %i show_fps: %i show_map: %i",
                     state->stop, state->show_fps, state->show_map);
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

        auto& kbd_input_mgr { state->kbd_input_mgr };
        std::sprintf(hud_line, "                    user input keys:");
        screen_buffer.replaceAtCoords(replace_col, 7,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "      down: %i right: %i up: %i left: %i",
                     kbd_input_mgr.isPressed(KEY_DOWN), kbd_input_mgr.isPressed(KEY_RIGHT),
                     kbd_input_mgr.isPressed(KEY_UP), kbd_input_mgr.isPressed(KEY_LEFT));
        screen_buffer.replaceAtCoords(replace_col, 8,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "                           a: %i d: %i",
                     kbd_input_mgr.isPressed(KEY_A), kbd_input_mgr.isPressed(KEY_D));
        screen_buffer.replaceAtCoords(replace_col, 9,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "                      p: %i m: %i f: %i",
                     kbd_input_mgr.isPressed(KEY_P), kbd_input_mgr.isPressed(KEY_M),
                     kbd_input_mgr.isPressed(KEY_F));
        screen_buffer.replaceAtCoords(replace_col, 10,
                                      hud_line_sz, hud_line);
        std::sprintf(hud_line, "       Lctrl: %i Rctrl: %i c: %i esc: %i",
                     kbd_input_mgr.isPressed(KEY_LEFTCTRL),
                     kbd_input_mgr.isPressed(KEY_RIGHTCTRL),
                     kbd_input_mgr.isPressed(KEY_C), kbd_input_mgr.isPressed(KEY_ESC));
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

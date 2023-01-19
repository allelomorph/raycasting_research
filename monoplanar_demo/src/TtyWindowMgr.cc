#include "TtyWindowMgr.hh"
#include "safeCExec.hh"               // C_*
#include "safeSdlExec.hh"             // SDL_RETURN_TEST
#include "Xterm.hh"                   // CtrlSeqs::

#include <SDL2/SDL_image.h>           // IMG_*

#include <termios.h>                  // winsize
#include <sys/ioctl.h>
#include <unistd.h>                   // ttyname STDIN_FILENO
#include <linux/input-event-codes.h>  // KEY_*

#include <cassert>
#include <cstdio>                     // sprintf
#include <cstring>                    // strlen memset

#include <iostream>
#include <iomanip>                    // setw
#include <string>
#include <sstream>
#include <algorithm>                  // max min


void TtyWindowMgr::renderAsciiPixelColumn(const uint16_t screen_x,
                                          const int16_t ceiling_screen_y,
                                          const uint16_t line_h,
                                          const WallOrientation wall_hit_algnmt) {
    uint16_t screen_y { 0 };
    uint16_t screen_line_begin_y ( std::max(0, (int)ceiling_screen_y) );
    uint16_t screen_line_end_y ( std::min((int)buffer.h, ceiling_screen_y + line_h) );
    uint16_t screen_w { buffer.w };
    TtyPixel* screen_px { buffer.pixel(screen_x, screen_y) };
    // draw ceiling
    for (; screen_y < screen_line_begin_y; ++screen_y, screen_px += screen_w)
        screen_px->c = ' ';
    // draw wall, shading NS walls darker to differentiate
    for (; screen_y < screen_line_end_y; ++screen_y, screen_px += screen_w) {
        screen_px->c =
            (wall_hit_algnmt == WallOrientation::NS) ? '|' : '@';
    }
    // draw floor
    for (; screen_y < buffer.h; ++screen_y, screen_px += screen_w)
        screen_px->c = ' ';
}

void TtyWindowMgr::render256ColorPixelColumn(const uint16_t screen_x,
                                             const int16_t ceiling_screen_y,
                                             const uint16_t line_h,
                                             const WallOrientation wall_hit_algnmt,
                                             const SDL_Surface* texture,
                                             const uint16_t tex_x) {
    uint16_t screen_y { 0 };
    uint16_t screen_line_begin_y ( std::max(0, (int)ceiling_screen_y) );
    uint16_t screen_line_end_y ( std::min((int)buffer.h, ceiling_screen_y + line_h) );
    uint16_t screen_w { buffer.w };
    TtyPixel* column_px { buffer.pixel(screen_x, screen_y) };
    // draw ceiling
    for (; screen_y < screen_line_begin_y; ++screen_y, column_px += screen_w) {
        column_px->code = (uint8_t)Xterm::Color::Codes::System::Black;
    }
    // draw wall, shading NS walls darker to differentiate
    double tex_h_ratio { texture->h / (double)line_h };
    SDL_PixelFormat* tex_format { texture->format };
    uint8_t* tex_px_data { (uint8_t*)(texture->pixels) + (tex_x * tex_format->BytesPerPixel) };
    uint16_t tex_row_sz ( texture->pitch );
    uint8_t r, g, b;
    for (uint16_t tex_y; screen_y < screen_line_end_y; ++screen_y, column_px += screen_w) {
        // buffer traversal can optimize away calling `buffer.pixel(
        //   screen_x, screen_y)` due a consistent step of screen_y += 1 each
        //   loop; using a similar approach to texture traversal is difficult
        //   as we need to accommodate any possible tex_h:line_h ratio, and
        //   so possibly inconsistent step values for tex_y as it is fit to screen_y
        tex_y = (screen_y - ceiling_screen_y /*line_y*/) * tex_h_ratio;
        SDL_GetRGB(*(uint32_t*)(tex_px_data + (tex_y * tex_row_sz)),
                   tex_format, &r, &g, &b);
        if (wall_hit_algnmt == WallOrientation::NS) {
            r /= 2;
            g /= 2;
            b /= 2;
        }
        column_px->code = Xterm::Color::Codes::fromRGB(r, g, b);
    }
    // draw floor
    for (; screen_y < buffer.h; ++screen_y, column_px += screen_w) {
        column_px->code = (uint8_t)Xterm::Color::Codes::System::Black;
    }
}

void TtyWindowMgr::renderTrueColorPixelColumn(const uint16_t screen_x,
                                              const int16_t ceiling_screen_y,
                                              const uint16_t line_h,
                                              const WallOrientation wall_hit_algnmt,
                                              const SDL_Surface* texture,
                                              const uint16_t tex_x) {
    uint16_t screen_y { 0 };
    uint16_t screen_line_begin_y ( std::max(0, (int)ceiling_screen_y) );
    uint16_t screen_line_end_y ( std::min((int)buffer.h, ceiling_screen_y + line_h) );
    uint16_t screen_w { buffer.w };
    TtyPixel* screen_px { buffer.pixel(screen_x, screen_y) };
    // draw ceiling
    for (; screen_y < screen_line_begin_y; ++screen_y, screen_px += screen_w) {
        screen_px->r = 0;
        screen_px->g = 0;
        screen_px->b = 0;
    }
    // draw wall, shading NS walls darker to differentiate
    double tex_h_ratio { texture->h / (double)line_h };
    SDL_PixelFormat* tex_format { texture->format };
    uint8_t* tex_px_data { (uint8_t*)(texture->pixels) + (tex_x * tex_format->BytesPerPixel) };
    uint16_t tex_row_sz ( texture->pitch );
    uint8_t r, g, b;
    for (uint16_t tex_y; screen_y < screen_line_end_y; ++screen_y, screen_px += screen_w) {
        // buffer traversal can optimize away calling `buffer.pixel(
        //   screen_x, screen_y)` due a consistent step of screen_y += 1 each
        //   loop; using a similar approach to texture traversal is difficult
        //   as we need to accommodate any possible tex_h:line_h ratio, and
        //   so possibly inconsistent step values for tex_y as it is fit to screen_y
        tex_y = (screen_y - ceiling_screen_y /*line_y*/) * tex_h_ratio;
        SDL_GetRGB(*(uint32_t*)(tex_px_data + (tex_y * tex_row_sz)),
                   tex_format, &r, &g, &b);
        if (wall_hit_algnmt == WallOrientation::NS) {
            r /= 2;
            g /= 2;
            b /= 2;
        }
        screen_px->r = r;
        screen_px->g = g;
        screen_px->b = b;
    }
    // draw floor
    for (; screen_y < buffer.h; ++screen_y, screen_px += screen_w) {
        screen_px->r = 0;
        screen_px->g = 0;
        screen_px->b = 0;
    }
}

// TBD: make public member? In case of building for optional multithreading,
//   each thread would have to trace a batch of rays, then render their
//   respective pixel columns without stopping to wait for the others,
//   which means each thread would have to call this.
// The core illusion of raycasting comes from rendering walls in vertical
//   strips, one per each ray cast in the FOV, with each strip being longer
//   as the ray is shorter/wall is closer, forcing perspective.
void TtyWindowMgr::renderPixelColumn(const uint16_t screen_x,
                                     const FovRay& ray,
                                     const TtyDisplayMode tty_display_mode) {

    // calculate height of vertical strip of wall to draw on screen
    uint16_t line_h ( buffer.h / ray.wall_hit.dist );

    // row index of highest pixel in strip (may be negative if camera is close
    //   to wall and wall unit does not fit in frame)
    int16_t ceiling_screen_y ( buffer.h / 2 - line_h / 2 );

    if (tty_display_mode == TtyDisplayMode::Ascii) {
        return renderAsciiPixelColumn(screen_x, ceiling_screen_y, line_h,
                                      ray.wall_hit.algnmt);
    }

    // TBD: add protection for out of range tex key? or in map parsing?
    // find proportionate x coordinate in wall texture
    SDL_Surface* texture { wall_texs.at(ray.wall_hit.tex_key).get() };
    uint16_t tex_x ( ray.wall_hit.x * texture->w );
    // ensure texture x of 0 is always to the left when facing the wall segment
    if ((ray.wall_hit.algnmt == WallOrientation::NS && ray.dir.x > 0) ||
        (ray.wall_hit.algnmt == WallOrientation::EW && ray.dir.y < 0) ) {
        tex_x = texture->w - tex_x - 1;
    }

    if (tty_display_mode == TtyDisplayMode::ColorCode) {
        return render256ColorPixelColumn(screen_x, ceiling_screen_y, line_h,
                                         ray.wall_hit.algnmt, texture, tex_x);
    }

    if (tty_display_mode == TtyDisplayMode::TrueColor) {
        return renderTrueColorPixelColumn(screen_x, ceiling_screen_y, line_h,
                                          ray.wall_hit.algnmt, texture, tex_x);
    }
}

TtyWindowMgr::TtyWindowMgr() {
    // SDL_Init in App()
    // image subsystem for texture loading
    safeSdlExec(IMG_Init, "IMG_Init", SDL_RETURN_TEST(int, (ret == 0)),
                IMG_INIT_JPG);
}

TtyWindowMgr::~TtyWindowMgr() {
    IMG_Quit();
    // SDL_Quit in ~App()
}

uint16_t TtyWindowMgr::width() { return buffer.w; }

uint16_t TtyWindowMgr::height() { return buffer.h; }

void TtyWindowMgr::resetBuffer() {
    buffer = TtyPixelBuffer(buffer.w, buffer.h);
}

void TtyWindowMgr::drawEmptyFrame() {
    std::cout << Xterm::CtrlSeqs::CursorHome() <<
        Xterm::CtrlSeqs::EraseLinesBelow();
}

void TtyWindowMgr::initialize(const Settings& settings,
                              const uint16_t layout_h) {
    // Use of ttyname taken from coreutils tty, see:
    //  - https://github.com/coreutils/coreutils/blob/master/src/tty.c
    tty_name = safeCExec(ttyname, "ttyname",
                         C_RETURN_TEST(char *, (ret == nullptr)),
                         STDIN_FILENO);

    fitToWindow(settings.map_proportion, layout_h);

    // TBD: eventually change to map of texture keys representing floor/ceiling/walls
    // dummmy texture at index 0, as map tile 0 represents non-wall tile
    wall_texs.emplace_back(SdlSurfaceUnqPtr(nullptr, surface_deleter));
    // load textures (into surfaces for per-pixel access)
    for (uint8_t i { 1 }; i < 9; ++i) {
        wall_texs.emplace_back( SdlSurfaceUnqPtr(
            safeSdlExec(IMG_Load, "IMG_Load", SDL_RETURN_TEST(SDL_Surface*, (ret == nullptr)),
                        wall_tex_paths[i]),
            surface_deleter) );
        std::cout << "Loaded texture: " << wall_tex_paths[i] << '\n';
    }

    // force scrollback of all terminal text by drawing an empty frame
    //   (buffer default init is to all black ' ' chars)
    // TBD: why both here?
    drawFrame(settings);
    drawEmptyFrame();
    std::cout << Xterm::CtrlSeqs::HideCursor();
}

void TtyWindowMgr::fitToWindow(const double map_proportion,
                               const uint16_t /*layout_h*/) {
    // get terminal window size in chars
    // use of TIOCGWINSZ taken from coreutils stty, see:
    //   - https://github.com/wertarbyte/coreutils/blob/master/src/stty.c#L1311
    struct winsize winsz;
    safeCExec(ioctl, "ioctl", C_RETURN_TEST(int, (ret == -1)),
              STDIN_FILENO, TIOCGWINSZ, &winsz);
    // replacing rather than resizing buffer due to stray map and hud chars not
    //   being overwritten when not in ascii mode
    buffer = TtyPixelBuffer(winsz.ws_col, winsz.ws_row);

    // set minimap w,h in chars
    assert(map_proportion > 0);
    minimap_h = buffer.h * map_proportion;
    // dims must be odd to center player icon
    if (minimap_h % 2 == 0)
        ++(minimap_h);
    minimap_w = (minimap_h * 2) + 1;
}

void TtyWindowMgr::renderView(const std::vector<FovRay>& fov_rays,
                               const Settings& settings) {
    TtyDisplayMode tty_display_mode { settings.tty_display_mode };
    for (uint16_t screen_x { 0 }; screen_x < buffer.w; ++screen_x) {
        renderPixelColumn(screen_x, fov_rays[screen_x], tty_display_mode);
    }
}

void TtyWindowMgr::renderMap(const DdaRaycastEngine& raycast_engine) {
    // assert(state->map_dims % 2);
    if (minimap_h < 5 || minimap_w >= buffer.w)
        return;
    std::string line;
    uint16_t bordered_map_w ( minimap_w + 2 );
    uint16_t window_col_i ( buffer.w - bordered_map_w );
    uint16_t window_row_i { 0 };
    // top border
    line.resize(bordered_map_w, ' ');
    buffer.pixelCharReplace(window_col_i, window_row_i,
                            line.c_str(), bordered_map_w);
    ++window_row_i;
    const uint16_t player_x ( raycast_engine.player_pos.x );
    const uint16_t player_y ( raycast_engine.player_pos.y );
    const uint16_t map_delta_y ( minimap_h / 2 );
    const uint16_t map_delta_x ( minimap_w / 2 );
    for (int16_t map_y ( player_y + map_delta_y );
         map_y >= player_y - map_delta_y; --map_y, ++window_row_i) {
        line.clear();
        line.push_back(' ');  // left border
        for (int16_t map_x ( player_x - map_delta_x );
             map_x <= player_x + map_delta_x; ++map_x) {
            if (map_x < 0 || map_y < 0 ||
                map_x >= static_cast<int16_t>(raycast_engine.layout.w) ||
                map_y >= static_cast<int16_t>(raycast_engine.layout.h) ||
                !raycast_engine.layout.tileIsWall(map_x, map_y)) {
                line.push_back(' ');
            } else {
                line.push_back(raycast_engine.layout.tile(map_x, map_y) + '0');
            }
        }
        line.push_back(' ');  // right border
        if (map_y == player_y) {
            char player_icon;
            double player_dir_angle { raycast_engine.player_dir.angle() };
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
        buffer.pixelCharReplace(window_col_i, window_row_i,
                                line.c_str(), bordered_map_w);
    }
    // bottom border
    line.clear();
    line.resize(bordered_map_w, ' ');
    buffer.pixelCharReplace(window_col_i, window_row_i,
                            line.c_str(), bordered_map_w);
}

void TtyWindowMgr::renderHud(const double pt_frame_duration_mvg_avg,
                             const double rt_frame_duration_mvg_avg,
                             const Settings& settings,
                             const DdaRaycastEngine& raycast_engine,
                             const KbdInputMgr* kbd_input_mgr) {
    // Using sprintf over idiomatic C++ to get exact precision on floating
    //   point values (not using C++20, so std::format is not an option.)
    char line[50] { '\0' };
    std::size_t line_sz;
    if (settings.show_fps || settings.debug_mode) {
        line_sz = std::sprintf(line, "PTFPS: %6.2f RTFPS: %6.2f ",
                               (1 / pt_frame_duration_mvg_avg),
                               (1 / rt_frame_duration_mvg_avg) );
        buffer.pixelCharReplace(0, 0, line, line_sz);
    }

    if (settings.debug_mode && buffer.h >= 10) {
        line_sz = std::sprintf(line, "show_fps(F1): %i show_map(F2): %i ",
                               settings.show_fps, settings.show_map);
        buffer.pixelCharReplace(0, 1, line, line_sz);
        line_sz = std::sprintf(line, "debug_mode(F3): %i euclidean(F4): %i ",
                               settings.debug_mode, settings.euclidean);
        buffer.pixelCharReplace(0, 2, line, line_sz);

        // -ddd.ddd format
        line_sz = std::sprintf(line, "player_pos: {%8.3f, %8.3f} ",
                               raycast_engine.player_pos.x,
                               raycast_engine.player_pos.y);
        buffer.pixelCharReplace(0, 3, line, line_sz);
        line_sz = std::sprintf(line, "player_dir: {%8.3f, %8.3f} ",
                               raycast_engine.player_dir.x,
                               raycast_engine.player_dir.y);
        buffer.pixelCharReplace(0, 4, line, line_sz);
        line_sz = std::sprintf(line, "view_plane: {%8.3f, %8.3f} ",
                               raycast_engine.view_plane.x,
                               raycast_engine.view_plane.y);
        buffer.pixelCharReplace(0, 5, line, line_sz);

        line_sz = std::sprintf(line, "window: %4uw : %4uh (%8.6f) ",
                               buffer.w, buffer.h, (double(buffer.w) / buffer.h));
        buffer.pixelCharReplace(0, 6, line, line_sz);

        line_sz = std::sprintf(line, "user input keys: ");
        buffer.pixelCharReplace(0, 7, line, line_sz);
        line_sz = std::sprintf(line, "down: %i right: %i up: %i left: %i ",
                               kbd_input_mgr->isPressed(KEY_DOWN),
                               kbd_input_mgr->isPressed(KEY_RIGHT),
                               kbd_input_mgr->isPressed(KEY_UP),
                               kbd_input_mgr->isPressed(KEY_LEFT));
        buffer.pixelCharReplace(0, 8, line, line_sz);
        line_sz = std::sprintf(line, "Lshft:%i Rshft:%i Lalt:%i Ralt:%i ",
                               kbd_input_mgr->isPressed(KEY_LEFTSHIFT),
                               kbd_input_mgr->isPressed(KEY_RIGHTSHIFT),
                               kbd_input_mgr->isPressed(KEY_LEFTALT),
                               kbd_input_mgr->isPressed(KEY_RIGHTALT));
        buffer.pixelCharReplace(0, 9, line, line_sz);
    }
}

// TBD: switch to pointer arithmetic optimization like renderPixelColumn*
// TBD: specialize this like renderPixelColumn to prevent testing tty_display_mode for every px
void TtyWindowMgr::drawFrame(const Settings& settings) {
    assert(buffer.h > 0);
    std::ostringstream oss;
    TtyPixel* px { buffer.pixel(0, 0) };
    const TtyDisplayMode tty_display_mode { settings.tty_display_mode };
    // subtraction implicitly converts to int
    uint16_t last_row_i ( buffer.h - 1 );
    if (tty_display_mode == TtyDisplayMode::Ascii) {
        for (uint16_t row_i { 0 }; row_i < last_row_i; ++row_i) {
            for (uint16_t col_i { 0 }; col_i < buffer.w; ++col_i, ++px) {
                oss << px->c;
            }
            oss << '\n';
        }
    } else if (tty_display_mode == TtyDisplayMode::ColorCode) {
        for (uint16_t row_i { 0 }; row_i < last_row_i; ++row_i) {
            for (uint16_t col_i { 0 }; col_i < buffer.w; ++col_i, ++px) {
                oss << Xterm::CtrlSeqs::CharBgColor(px->code) << px->c;
            }
            oss << Xterm::CtrlSeqs::CharDefaults() << '\n';
        }
    } else {
        for (uint16_t row_i { 0 }; row_i < last_row_i; ++row_i) {
            for (uint16_t col_i { 0 }; col_i < buffer.w; ++col_i, ++px) {
                oss << Xterm::CtrlSeqs::CharBgColor(px->r, px->g, px->b) << px->c;
            }
            oss << Xterm::CtrlSeqs::CharDefaults() << '\n';
        }
    }
    // TBD: last line could instead be used for notifications and collecting
    //   user text input, eg loading a new map file
    // newline in last row would scroll screen up
    for (uint16_t col_i { 0 }; col_i < buffer.w; ++col_i, ++px) {
        if (tty_display_mode == TtyDisplayMode::ColorCode)
            oss << Xterm::CtrlSeqs::CharBgColor(px->code);
        else if (tty_display_mode == TtyDisplayMode::TrueColor)
            oss << Xterm::CtrlSeqs::CharBgColor(px->r, px->g, px->b);
        oss << px->c;
    }

    std::cout << oss.str();
    std::cout << Xterm::CtrlSeqs::CharDefaults() << Xterm::CtrlSeqs::CursorHome();
}

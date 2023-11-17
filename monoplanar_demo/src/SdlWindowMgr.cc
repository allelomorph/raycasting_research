#include "SdlWindowMgr.hh"
#include "safeSdlCall.hh"      // SDL_RETURN_TEST
#include "sdl_unique_ptrs.hh"  // *UnqPtr

#include <SDL2/SDL_image.h>    // IMG_*
#include <SDL2/SDL_ttf.h>      // TTF_*

#include <string>
#include <algorithm>           // min
#include <iostream>

#include <cassert>


void SdlWindowMgr::makeGlyphs(const char* font_filename) {
    // reference for font size ratio is 16pt on 480p window, or 1/30 window_h
    SdlTtfFontUnqPtr font (
        safeSdlCall(TTF_OpenFont, "TTF_OpenFont",
                    SDL_RETURN_TEST(TTF_Font*, ret == nullptr),
                    font_filename, window_h / 30 /*ptsize*/),
        ttf_font_deleter);
    SDL_Color fg { 0xff, 0xff, 0xff, 0xff };  // text foreground color (opaque white)
    TTF_Font* _font { font.get() };
    SDL_Renderer* _renderer { renderer.get() };

    // (no need to font_cache.clear(), as populating same set of keys every time,
    //   and SdlTextureUnqPtr dtors called on reassignment)
    // fill font cache with set of all printable ascii chars
    for (char c { ' ' }; c <= '~'; ++c) {
        SdlSurfaceUnqPtr glyph_surface (
            safeSdlCall(TTF_RenderGlyph_Blended, "TTF_RenderGlyph_Blended",
                        SDL_RETURN_TEST(SDL_Surface*, ret == nullptr),
                        _font, c, fg),
            surface_deleter);
        font_cache[c] = SdlTextureUnqPtr (
            safeSdlCall(SDL_CreateTextureFromSurface, "SDL_CreateTextureFromSurface",
                        SDL_RETURN_TEST(SDL_Texture*, ret == nullptr),
                        _renderer, glyph_surface.get()),
            texture_deleter);
    }
}

void SdlWindowMgr::renderHudLine(const std::string line, SDL_Rect glyph_rect) {
    SDL_Renderer* _renderer { renderer.get() };
    for (const char c : line) {
        SDL_Texture* glyph_tex { font_cache.at(c).get() };
        SDL_RenderFillRect(_renderer, &glyph_rect);
        SDL_RenderCopy(_renderer, glyph_tex, nullptr, &glyph_rect);
        glyph_rect.x += glyph_rect.w;
    }
}

// TBD: make public member? In case of building for optional multithreading,
//   each thread would have to trace a batch of rays, then render their
//   respective pixel columns without stopping to wait for the others,
//   which means each thread would have to call this.
// The core illusion of raycasting comes from rendering walls in vertical
//   strips, one per each ray cast in the FOV, with each strip being longer
//   as the ray is shorter/wall is closer, forcing perspective.
void SdlWindowMgr::renderPixelColumn(const uint16_t screen_x,
                                      const FovRay& ray) {

    // calculate height of vertical strip of wall to draw on screen
    uint16_t line_h ( window_h / ray.wall_hit.dist );

    // row index of highest pixel in strip (may be negative if camera is close
    //   to wall and wall unit does not fit in frame)
    int16_t ceiling_screen_y ( window_h / 2 - line_h / 2 );

    // TBD: add protection for out of range tex key? or in map parsing?
    // find proportionate x coordinate in wall texture
    SDL_Surface* texture { wall_texs.at(ray.wall_hit.tex_key).get() };
    uint16_t tex_x ( ray.wall_hit.x * texture->w );
    // ensure texture x of 0 is always to the left when facing the wall segment
    if ((ray.wall_hit.algnmt == WallOrientation::NS && ray.dir.x > 0) ||
        (ray.wall_hit.algnmt == WallOrientation::EW && ray.dir.y < 0) ) {
        tex_x = texture->w - tex_x - 1;
    }

    uint16_t screen_y { 0 };
    uint16_t screen_line_begin_y ( std::max(0, (int)ceiling_screen_y) );
    uint16_t screen_line_end_y ( std::min((int)window_h, ceiling_screen_y + line_h) );
    SDL_PixelFormat* screen_format { buffer->format };
    uint8_t* screen_px_data { surfacePixelPtr(buffer.get(), screen_x, screen_y) };
    uint16_t screen_row_sz ( buffer->pitch );
    // draw transparent ceiling
    for (; screen_y < screen_line_begin_y; ++screen_y, screen_px_data += screen_row_sz) {
        // TBD: more efficient to simply set alpha to 0?
        *((uint32_t*)screen_px_data) = 0;
    }
    // draw wall, shading NS walls darker to differentiate
    double tex_h_ratio { texture->h / (double)line_h };
    SDL_PixelFormat* tex_format { texture->format };
    uint8_t* tex_px_data { surfacePixelPtr(texture, tex_x, 0) };
    uint16_t tex_row_sz ( texture->pitch );
    uint8_t r, g, b;
    for (uint16_t tex_y;
         screen_y < screen_line_end_y; ++screen_y, screen_px_data += screen_row_sz) {
        // screen buffer traversal can optimize away calling surfacePixelPtr
        //   due a consistent step of screen_y += 1 each loop; using a similar
        //   approach to texture traversal is difficult as we need to
        //   accommodate any possible tex_h:line_h ratio, and so possibly
        //   inconsistent step values for tex_y as it is fit to screen_y
        tex_y = (screen_y - ceiling_screen_y /*line_y*/) * tex_h_ratio;
        SDL_GetRGB(*(uint32_t*)(tex_px_data + (tex_y * tex_row_sz)),
                   tex_format, &r, &g, &b);
        if (ray.wall_hit.algnmt == WallOrientation::NS) {
            r /= 2;
            g /= 2;
            b /= 2;
        }
        *((uint32_t*)screen_px_data) = SDL_MapRGBA(
            screen_format, r, g, b, SDL_ALPHA_OPAQUE);
    }
    // draw transparent floor
    for (; screen_y < window_h; ++screen_y, screen_px_data += screen_row_sz) {
        // TBD: more efficient to simply set alpha to 0?
        *((uint32_t*)screen_px_data) = 0;
    }
}

SdlWindowMgr::SdlWindowMgr() {
    // SDL_Init in App()
    // image subsystem for texture loading
    safeSdlCall(IMG_Init, "IMG_Init", SDL_RETURN_TEST(int, (ret == 0)),
                IMG_INIT_JPG);
    // tff font subsystem for fps text
    safeSdlCall(TTF_Init, "TTF_Init", SDL_RETURN_TEST(int, (ret == -1)) );
}

SdlWindowMgr::~SdlWindowMgr() {
    TTF_Quit();
    IMG_Quit();
    // SDL_Quit in ~App()
}

uint32_t SdlWindowMgr::id() {
    return safeSdlCall(SDL_GetWindowID, "SDL_GetWindowID",
                       SDL_RETURN_TEST(uint32_t, ret == 0),
                       window.get());
}

uint16_t SdlWindowMgr::width() { return window_w; }

uint16_t SdlWindowMgr::height() { return window_h; }

void SdlWindowMgr::initialize(const Settings& settings,
                               const uint16_t layout_h) {
    //
    // init window and window buffer
    //
    window = SdlWindowUnqPtr(
        safeSdlCall(SDL_CreateWindow, "SDL_CreateWindow",
                    SDL_RETURN_TEST(SDL_Window*, ret == nullptr),
                    "monoplanar raycast demo" /*name*/,
                    SDL_WINDOWPOS_CENTERED /*x*/, SDL_WINDOWPOS_CENTERED /*y*/,
                    WINDOW_WIDTH, WINDOW_HEIGHT,
                    SDL_WINDOW_RESIZABLE /*flags*/),
        window_deleter);

    // TBD: can we set the renderer blendmode once instead of individual texture
    //   blendmodes? SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND)
    //   ( == 0 on success)
    renderer = SdlRendererUnqPtr(
        safeSdlCall(SDL_CreateRenderer, "SDL_CreateRenderer",
                    SDL_RETURN_TEST(SDL_Renderer*, ret == nullptr),
                    window.get(), -1 /*driver index (first to support flags)*/,
                    SDL_RENDERER_ACCELERATED /*flags*/),
        renderer_deleter);

    // also populates font cache textures
    fitToWindow(settings.map_proportion, layout_h);

    //
    // init textures other than font cache
    //
    SdlSurfaceUnqPtr sky_surface (
        safeSdlCall(IMG_Load, "IMG_Load",
                    SDL_RETURN_TEST(SDL_Surface*, (ret == nullptr)),
                    SKY_TEX_PATH),
        surface_deleter);
    std::cout << "Loaded texture: " << SKY_TEX_PATH << '\n';
    sky_tex = SdlTextureUnqPtr(
        safeSdlCall(SDL_CreateTextureFromSurface, "SDL_CreateTextureFromSurface",
                    SDL_RETURN_TEST(SDL_Texture*, (ret == nullptr)),
                    renderer.get(), sky_surface.get()),
        texture_deleter);
    // set to alpha blending (for fps glyphs)
    safeSdlCall(SDL_SetTextureBlendMode, "SDL_SetTextureBlendMode",
                SDL_RETURN_TEST(int, ret < 0),
                sky_tex.get(), SDL_BLENDMODE_BLEND);
    // TBD: eventually change to map of texture keys representing floor/ceiling/walls
    // dummmy texture at index 0, as map tile 0 represents non-wall tile
    wall_texs.emplace_back(SdlSurfaceUnqPtr(nullptr, surface_deleter));
    // load textures (into surfaces for per-pixel access)
    for (uint8_t i { 1 }; i < 9; ++i) {
        wall_texs.emplace_back( SdlSurfaceUnqPtr(
            safeSdlCall(IMG_Load, "IMG_Load",
                        SDL_RETURN_TEST(SDL_Surface*, (ret == nullptr)),
                        wall_tex_paths[i]),
            surface_deleter) );
        std::cout << "Loaded texture: " << wall_tex_paths[i] << '\n';
    }
}

void SdlWindowMgr::fitToWindow(const double map_proportion,
                                const uint16_t layout_h) {
    assert(window.get() != nullptr);
    int w, h;
    SDL_GetWindowSize(window.get(), &w, &h);  // void return, no error checking
    window_w = w;
    window_h = h;

    // using format with alpha channel for blending of fps glyphs
    buffer = SdlSurfaceUnqPtr(
        safeSdlCall(SDL_CreateRGBSurfaceWithFormat, "SDL_CreateRGBSurfaceWithFormat",
                    SDL_RETURN_TEST(SDL_Surface*, ret == nullptr),
                    0 /*flags*/, window_w, window_h,
                    32 /*depth (bits per pixel)*/, SDL_PIXELFORMAT_BGRA32),
        surface_deleter);
    // Even if pixel setting is multithreaded, no two threads should be
    //   accessing the same pixel column at once, so we can rule out use of
    //   SDL_LockSurface/SDL_UnlockSurface to improve performance
    assert(!SDL_MUSTLOCK(buffer.get()));
    buffer_tex = SdlTextureUnqPtr(
        safeSdlCall(SDL_CreateTexture, "SDL_CreateTexture",
                    SDL_RETURN_TEST(SDL_Texture*, ret == nullptr),
                    renderer.get(), buffer->format->format,
                    SDL_TEXTUREACCESS_STREAMING /*flags*/,
                    window_w, window_h),
        texture_deleter);
    // set to alpha blending (for fps glyphs)
    safeSdlCall(SDL_SetTextureBlendMode, "SDL_SetTextureBlendMode",
                SDL_RETURN_TEST(int, ret < 0),
                buffer_tex.get(), SDL_BLENDMODE_BLEND);

    minimap_scale = (window_h * map_proportion) / layout_h;
    minimap_viewport.w = minimap_scale * 25;
    minimap_viewport.h = minimap_scale * 25;
    // upper right minimap
    minimap_viewport.x = window_w - minimap_viewport.w;
    minimap_viewport.y = 0;

    makeGlyphs(FONT_PATH);
}

void SdlWindowMgr::renderView(const std::vector<FovRay>& fov_rays,
                               const Settings& /*settings*/) {
    // TBD: currently rendering entire skyplane and then drawing walls on top
    //   perhaps we can make viewport on skyplane so it's not stretched, and then
    //   set buffer pixels of only sky above walls, and floor below
    // render entire skyplane tx to window, stretch to fit
    SDL_RenderCopy(renderer.get(), sky_tex.get(), nullptr, nullptr);
    for (uint16_t window_x { 0 }; window_x < window_w; ++window_x) {
        renderPixelColumn(window_x, fov_rays[window_x]);
    }
    SDL_UpdateTexture(buffer_tex.get(), nullptr, buffer->pixels, buffer->pitch);
    // fit entire texture to window
    SDL_RenderCopy(renderer.get(), buffer_tex.get(), nullptr, nullptr);
}

void SdlWindowMgr::renderMap(const DdaRaycastEngine& raycast_engine) {
    SDL_Renderer* _renderer { renderer.get() };
    // render to entire window
    SDL_RenderSetViewport(_renderer, nullptr);
    // single pixel scale
    SDL_RenderSetScale(_renderer, 1, 1);
    SDL_SetRenderDrawColor(_renderer, 0x4B, 0x4B, 0x4B, SDL_ALPHA_OPAQUE);
    // fill in minimap background with dark grey
    SDL_RenderFillRect(_renderer, &minimap_viewport);

    SDL_RenderSetViewport(_renderer, &minimap_viewport);
    // interpret integers as minimap grid units
    SDL_RenderSetScale(_renderer, minimap_scale, minimap_scale);
    SDL_SetRenderDrawColor(_renderer, 0x8F, 0x8F, 0x8F, SDL_ALPHA_OPAQUE);
    // empty map coordinates drawn in light grey; walls implied via negative space
    const Layout& layout { raycast_engine.layout };
    SDL_Rect map_tile { /*x*/0, /*y*/0, /*w*/1, /*h*/1 };
    const uint16_t player_x ( raycast_engine.player_pos.x );
    const uint16_t player_y ( raycast_engine.player_pos.y );
    const uint16_t map_delta ( MINIMAP_GRID_SZ / 2 );
    for (int16_t map_y ( player_y + map_delta );
         map_y >= player_y - map_delta; --map_y, map_tile.x = 0, ++map_tile.y) {
        for (int16_t map_x ( player_x - map_delta );
             map_x <= player_x + map_delta; ++map_x, ++map_tile.x) {
            if (map_x < 0 || map_y < 0 ||
                map_x >= static_cast<int16_t>(layout.w) ||
                map_y >= static_cast<int16_t>(layout.h) ||
                !layout.tileIsWall(map_x, map_y)) {
                SDL_RenderFillRect(_renderer, &map_tile);
            }
        }
    }
    // scale dot to .5 grid unit
    SDL_RenderSetScale(_renderer, minimap_scale / 2, minimap_scale / 2);
    SDL_SetRenderDrawColor(_renderer, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    // draw player as red dot at (center x, center y)
    SDL_RenderDrawPoint(_renderer, /*(*/MINIMAP_GRID_SZ/* / 2) * 2*/,
                        /*(*/MINIMAP_GRID_SZ/* / 2) * 2*/);

    // return to default renderer vp and scale (full window, scale 1:1)
    SDL_RenderSetViewport(_renderer, nullptr);
    SDL_RenderSetScale(_renderer, 1, 1);
}


void SdlWindowMgr::renderHud(const double pt_frame_duration_mvg_avg,
                              const double rt_frame_duration_mvg_avg,
                              const Settings& settings,
                              const DdaRaycastEngine& raycast_engine,
                              const KbdInputMgr* kbd_input_mgr) {
    char line[50] { '\0' };
    SDL_Rect glyph_rect { 0, 0, 0, 0 };
    // font "Courier New.ttf" is monospaced, so should have consistent glyph
    //   texture width
    SDL_QueryTexture(font_cache.at('0').get(), nullptr, nullptr,
                     &glyph_rect.w, &glyph_rect.h);
    // transparent black glyph background
    SDL_SetRenderDrawColor(renderer.get(), 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);

    if (settings.show_fps || settings.debug_mode) {
        std::sprintf(line, "PTFPS: %6.2f RTFPS: %6.2f",
                     (1 / pt_frame_duration_mvg_avg),
                     (1 / rt_frame_duration_mvg_avg) );
        renderHudLine(line, glyph_rect);
    }

    if (settings.debug_mode) {
        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "show_fps(F1): %i show_map(F2): %i",
                     settings.show_fps, settings.show_map);
        renderHudLine(line, glyph_rect);
        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "debug_mode(F3): %i euclidean(F4): %i",
                     settings.debug_mode, settings.euclidean);
        renderHudLine(line, glyph_rect);

        // -ddd.ddd format
        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "player_pos: {%8.3f, %8.3f}",
                     raycast_engine.player_pos.x,
                     raycast_engine.player_pos.y);
        renderHudLine(line, glyph_rect);
        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "player_dir: {%8.3f, %8.3f}",
                     raycast_engine.player_dir.x,
                     raycast_engine.player_dir.y);
        renderHudLine(line, glyph_rect);
        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "view_plane: {%8.3f, %8.3f}",
                     raycast_engine.view_plane.x,
                     raycast_engine.view_plane.y);
        renderHudLine(line, glyph_rect);

        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "window: %4uw : %4uh (%8.6f)",
                     window_w, window_h, (double(window_w) / window_h));
        renderHudLine(line, glyph_rect);

        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "user input keys:");
        renderHudLine(line, glyph_rect);
        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "down: %i right: %i up: %i left: %i",
                     kbd_input_mgr->isPressed(SDLK_DOWN),
                     kbd_input_mgr->isPressed(SDLK_RIGHT),
                     kbd_input_mgr->isPressed(SDLK_UP),
                     kbd_input_mgr->isPressed(SDLK_LEFT));
        renderHudLine(line, glyph_rect);
        glyph_rect.y += glyph_rect.h;
        std::sprintf(line, "Lshft:%i Rshft:%i Lalt:%i Ralt:%i",
                     kbd_input_mgr->isPressed(SDLK_LSHIFT),
                     kbd_input_mgr->isPressed(SDLK_RSHIFT),
                     kbd_input_mgr->isPressed(SDLK_LALT),
                     kbd_input_mgr->isPressed(SDLK_RALT));
        renderHudLine(line, glyph_rect);
    }
}

void SdlWindowMgr::drawFrame(const Settings& /*settings*/) {
    SDL_RenderPresent(renderer.get());
}

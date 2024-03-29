#ifndef SDLWINDOWMGR_HH
#define SDLWINDOWMGR_HH

#include "DdaRaycastEngine.hh"    // FovRay
#include "KbdInputMgr.hh"
#include "Settings.hh"
#include "WindowMgr.hh"           // sdl2_unq

#include "sdl2_ttf_smart_ptr.hh"  // sdl2_smart_ptr::unique::TtfFont

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>

#include <cstdint>

#include <vector>
#include <unordered_map>
#include <string>


class SdlWindowMgr : public WindowMgr {
private:
    static constexpr uint16_t WINDOW_HEIGHT { 480 };
    static constexpr uint16_t WINDOW_WIDTH { 853 };  // 853:480 ~ 16:9
    static constexpr char SKY_TEX_PATH[] { "images/Vue1.jpg" };
    static constexpr char FONT_PATH[] { "fonts/Courier New.ttf" };

    // Best way in testing to prevent leaks and read errors with the freeing of
    //   a SDL window-renderer-texture association was to free in the reverse
    //   order of allocation (texture-renderer-window,) which should be the
    //   automatic behavior of member unique_ptr dtors (and thus deleters)
    //   being called in reverse order of declaration.
    // X11 window
    sdl2_unq::Window   window;
    // window renderer
    sdl2_unq::Renderer renderer;
    // full window texture created from buffer, to render as video frame
    sdl2_unq::Texture  buffer_tex;
    // main surface that is drawn to by game engine
    sdl2_unq::Surface  buffer;

    uint16_t window_w;
    uint16_t window_h;

    // element textures
    //
    // sky plane (maze background when not texturing ceiling and floor)
    sdl2_unq::Texture               sky_tex;
    // wall textures (SDL_Surface instead of SDL_Texture for per-pixel access)
    std::vector<sdl2_unq::Surface>  wall_texs;
    // HUD chars
    std::unordered_map<
        uint8_t, sdl2_unq::Texture> font_cache;

    // minimap rendering
    //
    // size of minimap grid unit in pixels
    float minimap_scale;
    // map units per minimap side
    static constexpr uint16_t MINIMAP_GRID_SZ { 25 };
    SDL_Rect minimap_viewport;

    inline uint8_t* surfacePixelPtr(SDL_Surface* sp, const uint16_t screen_x,
                                    const uint16_t screen_y) {
        // assert(sp != nullptr && screen_x < sp->w && screen_y < sp->h);
        return (uint8_t*)(sp->pixels) + (screen_y * sp->pitch) +
            (screen_x * sp->format->BytesPerPixel);
    };
    // cache TrueType font textures for printable ASCII, scaled to window_h
    void makeGlyphs(const char* font_filename);
    // render formatted line of text
    void renderHudLine(const std::string line, SDL_Rect glyph_rect);
    // render one vertical wall segment
    void renderPixelColumn(const uint16_t screen_x, const FovRay& ray);

public:
    SdlWindowMgr();
    ~SdlWindowMgr();

    uint32_t id();
    uint16_t width();
    uint16_t height();

    void initialize(const Settings& settings, const uint16_t layout_h);

    // adjusting rendering specs to after window resize event
    void fitToWindow(const double map_proportion, const uint16_t layout_h);

    void renderView(const std::vector<FovRay>& fov_rays,
                    const Settings& /*settings*/);

    void renderMap(const DdaRaycastEngine& raycast_engine);

    void renderHud(const double pt_frame_duration_mvg_avg,
                   const double rt_frame_duration_mvg_avg,
                   const Settings& settings,
                   const DdaRaycastEngine& raycast_engine,
                   const KbdInputMgr* kbd_input_mgr);

    void drawFrame(const Settings& settings);
};

// C++11 static constexpr members that are not built-in types need redeclaration
//   outside class: https://en.cppreference.com/w/cpp/language/static
//constexpr char SdlWindowMgr::SKY_TEX_PATH[];
//constexpr char SdlWindowMgr::FONT_PATH[];


#endif  // SDLWINDOWMGR_HH

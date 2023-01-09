#ifndef SDLDISPLAYMGR_HH
#define SDLDISPLAYMGR_HH

//#include "DisplayMgr.hh"
#include "Settings.hh"
#include "DdaRaycastEngine.hh"  // FovRay
#include "KbdInputMgr.hh"
#include "sdl_unique_ptrs.hh"   // SdlDeleter::* Sdl*UnqPtr

#include <cstdint>

#include <array>
#include <vector>


// ~16:9 at 480p
constexpr uint16_t WINDOW_HEIGHT { 480 };
constexpr uint16_t WINDOW_WIDTH { 853 };

// TBD: put in parent class
// ordered so that indices match texture keys in wall_textures
constexpr std::array<const char*, 10> wall_texture_paths {
    "",
    "../images/wood.jpg",
    "../images/metal.jpg",
    "../images/curtain.jpg",
    "../images/stone_moss.jpg",
    "../images/bark.jpg",
    "../images/privat_parkering.jpg",
    "../images/grass.jpg",
    "../images/lava.jpg",
    ""
};

constexpr char SKY_TEXTURE_PATH[] { "../images/Vue1.jpg" };

constexpr char FONT_PATH[] { "../fonts/Courier New.ttf" };

class SdlDisplayMgr /*: public DisplayMgr */ {
private:
    // functors for SDL struct pointer deallocations
    //
    const SdlDeleter::Window   window_deleter {};
    const SdlDeleter::Renderer renderer_deleter {};
    const SdlDeleter::Surface  surface_deleter {};
    const SdlDeleter::Texture  texture_deleter {};
    const SdlDeleter::TtfFont  ttf_font_deleter {};

    // Best way in testing to prevent leaks and read errors with the freeing of
    //   a SDL window-renderer-texture association was to free in the reverse
    //   order of allocation (texture-renderer-window,) which should be the
    //   automatic behavior of member unique_ptr dtors (and thus deleters)
    //   being called in reverse order of declaration.
    // X11 window
    SdlWindowUnqPtr                window;
    // window renderer
    SdlRendererUnqPtr              renderer;
    // full window texture created from buffer, to render as video frame
    SdlTextureUnqPtr               buffer_tx;
    // main surface that is drawn to by game engine
    SdlSurfaceUnqPtr               buffer;

    // element textures
    //
    // sky plane (maze background when not texturing ceiling and floor)
    SdlTextureUnqPtr               sky_tx;
    // wall textures (SDL_Surface instead of SDL_Texture for per-pixel access)
    std::vector<SdlSurfaceUnqPtr>  wall_txs;
    // HUD chars
    std::unordered_map<
        uint8_t, SdlTextureUnqPtr> font_cache;

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

    void renderPixelColumn(const uint16_t screen_x, const FovRay& ray);

    void makeGlyphs(const char* font_filename);

    void renderHudLine(const std::string line, SDL_Rect glyph_rect);

public:
    uint16_t window_w;
    uint16_t window_h;

    SdlDisplayMgr();
    ~SdlDisplayMgr();

    void initialize(const Settings& settings,
                    const uint16_t layout_w, const uint16_t layout_h);

    // adjusting to SDL window resize
    void fitToWindow(const double map_proportion,
                     const uint16_t layout_w, const uint16_t layout_h);

    // TBD: find way to remove or implement cD and rB in parent class
    void resetBuffer() {}
    void clearDisplay() {}

    uint32_t getWindowId();

    void renderView(const std::vector<FovRay>& fov_rays, const Settings& settings);

    void renderMap(const DdaRaycastEngine& raycast_engine);

    void renderHUD(const double pt_frame_duration_mvg_avg,
                   const double rt_frame_duration_mvg_avg,
                   const Settings& settings,
                   const DdaRaycastEngine& raycast_engine,
                   const std::unique_ptr<KbdInputMgr>& kbd_input_mgr);

    void drawScreen(const Settings& settings);
};

#endif  // SDLDISPLAYMGR_HH

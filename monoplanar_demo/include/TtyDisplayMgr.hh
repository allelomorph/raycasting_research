#ifndef TTYDISPLAYMGR_HH
#define TTYDISPLAYMGR_HH

// #include "DisplayMgr.hh"
#include "TtyScreenBuffer.hh"
#include "Settings.hh"          // TtyDisplayMode
#include "DdaRaycastEngine.hh"  // FovRay
#include "KbdInputMgr.hh"
#include "sdl_unique_ptrs.hh"   // SdlDeleter::Surface SdlSurfaceUnqPtr

#include <SDL2/SDL_surface.h>

#include <csignal>              // sig_atomic_t
#include <cstdint>              // uint16_t

#include <string>
#include <vector>
#include <array>
#include <memory>               // unique_ptr


// TBD: establish minimum viable display dimensions in chars (original was 1080 x 640px in SDL)
constexpr std::array<const char*, 9> wall_texture_paths {
    "",
    "../images/wood.jpg",
    "../images/metal.jpg",
    "../images/curtain.jpg",
    "../images/stone_moss.jpg",
    "../images/bark.jpg",
    "../images/privat_parkering.jpg",
    "../images/grass.jpg",
    "../images/lava.jpg"
};

// TBD: move all wall_textures related code to parent class

class TtyDisplayMgr /*: public DisplayMgr */ {
private:
    TtyScreenBuffer screen_buffer;

    // TBD: move SDL deleter/unique_ptrs to header file?
    const SdlDeleter::Surface surface_deleter {};

    std::vector<SdlSurfaceUnqPtr> wall_textures;

    void updateMiniMapSize(const double map_proportion);

    void renderAsciiPixelColumn(const uint16_t screen_x,
                                const int16_t ceiling_screen_y,
                                const uint16_t line_h,
                                const WallOrientation wall_hit_algnmt);

    void render256ColorPixelColumn(const uint16_t screen_x,
                                   const int16_t ceiling_screen_y,
                                   const uint16_t line_h,
                                   const WallOrientation wall_hit_algnmt,
                                   const SDL_Surface* texture,
                                   const uint16_t tex_x);

    void renderTrueColorPixelColumn(const uint16_t screen_x,
                                    const int16_t ceiling_screen_y,
                                    const uint16_t line_h,
                                    const WallOrientation wall_hit_algnmt,
                                    const SDL_Surface* texture,
                                    const uint16_t tex_x);

    void renderPixelColumn(const uint16_t screen_x, const FovRay& ray,
                           const TtyDisplayMode tty_display_mode);

// TBD: will be inherited from DisplayMgr
protected:
    uint16_t minimap_w;
    uint16_t minimap_h;

public:
    std::string tty_name;

    TtyDisplayMgr();
    ~TtyDisplayMgr();

    void initialize(const Settings& settings);

    void fitToWindow(const double map_proportion);

    inline uint16_t screenWidth() { return screen_buffer.w; }

    // TBD: resetBuffer and clearDisplay need to be addressed in DiplayMgr interface
    void resetBuffer();

    void clearDisplay();

    void renderView(const std::vector<FovRay>& fov_rays,
                    const Settings& settings);

    void renderMap(const DdaRaycastEngine& raycast_engine);

    void renderHUD(const double pt_frame_duration_mvg_avg,
                   const double rt_frame_duration_mvg_avg,
                   const Settings& settings,
                   const DdaRaycastEngine& raycast_engine,
                   const std::unique_ptr<KbdInputMgr>& kbd_input_mgr);

    // TBD: drawFrame instead?
    void drawScreen(const Settings& settings);
};

#endif  // TTYDISPLAYMGR_HH

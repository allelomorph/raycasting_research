#ifndef WINDOWMGR_HH
#define WINDOWMGR_HH

#include "sdl_unique_ptrs.hh"   // SdlDeleter::* Sdl*UnqPtr
#include "Settings.hh"
#include "DdaRaycastEngine.hh"  // FovRay
#include "KbdInputMgr.hh"

#include <cstdint>

#include <vector>
#include <array>


class WindowMgr {
protected:
    const SdlDeleter::Surface surface_deleter {};

    // wall textures (SDL_Surface instead of SDL_Texture for per-pixel access)
    std::vector<SdlSurfaceUnqPtr> wall_texs;

    // ordered so that indices match texture indices in wall_texs
    static constexpr std::array<const char*, 10> wall_tex_paths {
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

public:
    virtual uint32_t id() { return 0; }
    virtual uint16_t width() = 0;
    virtual uint16_t height() = 0;

    virtual void resetBuffer() {}
    virtual void drawEmptyFrame() {}

    virtual void initialize(const Settings& settings,
                            const uint16_t layout_h) = 0;

    virtual void fitToWindow(const double map_proportion,
                             const uint16_t layout_h) = 0;

    virtual void renderView(const std::vector<FovRay>& fov_rays,
                            const Settings& settings) = 0;

    virtual void renderMap(const DdaRaycastEngine& raycast_engine) = 0;

    // TBD: change to KbdInputMgr*?
    virtual void renderHud(const double pt_frame_duration_mvg_avg,
                           const double rt_frame_duration_mvg_avg,
                           const Settings& settings,
                           const DdaRaycastEngine& raycast_engine,
                           const KbdInputMgr* kbd_input_mgr) = 0;

    virtual void drawFrame(const Settings& settings) = 0;
};

// C++11 static constexpr members that are not built-in types need redeclaration
//   outside class: https://en.cppreference.com/w/cpp/language/static
//constexpr std::array<const char*, 10> WindowMgr::wall_tex_paths;


#endif  // WINDOWMGR_HH

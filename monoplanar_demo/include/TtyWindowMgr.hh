#ifndef TTYWINDOWMGR_HH
#define TTYWINDOWMGR_HH

#include "WindowMgr.hh"
#include "TtyPixelBuffer.hh"
#include "DdaRaycastEngine.hh"  // WallOrientation FovRay
#include "Settings.hh"          // TtyDisplayMode
#include "KbdInputMgr.hh"

#include <SDL2/SDL_surface.h>

#include <cstdint>

#include <string>
#include <vector>


class TtyWindowMgr : public WindowMgr {
private:
    TtyPixelBuffer buffer;

    uint16_t minimap_w;
    uint16_t minimap_h;

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

public:
    std::string tty_name;

    TtyWindowMgr();
    ~TtyWindowMgr();

    uint16_t width();
    uint16_t height();

    void resetBuffer();
    void drawEmptyFrame();

    void initialize(const Settings& settings, const uint16_t layout_h);

    void fitToWindow(const double map_proportion, const uint16_t /*layout_h*/);

    void renderView(const std::vector<FovRay>& fov_rays,
                    const Settings& settings);

    void renderMap(const DdaRaycastEngine& raycast_engine);

    // TBD: change to KbdInputMgr*?
    void renderHud(const double pt_frame_duration_mvg_avg,
                   const double rt_frame_duration_mvg_avg,
                   const Settings& settings,
                   const DdaRaycastEngine& raycast_engine,
                   const KbdInputMgr* kbd_input_mgr);

    void drawFrame(const Settings& settings);
};


#endif  // TTYWINDOWMGR_HH

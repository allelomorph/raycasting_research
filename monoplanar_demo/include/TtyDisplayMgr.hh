#ifndef TTYDISPLAYMGR_HH
#define TTYDISPLAYMGR_HH

// #include "DisplayMgr.hh"
#include "TtyScreenBuffer.hh"
#include "Settings.hh"          // TtyDisplayMode
#include "DdaRaycastEngine.hh"  // FovRay
#include "KbdInputMgr.hh"

#include <csignal>              // sig_atomic_t
#include <cstdint>              // uint16_t

#include <string>
#include <vector>
#include <memory>               // unique_ptr


// TBD: establish minimum viable display dimensions in chars (original was 1080 x 640px in SDL)

class TtyDisplayMgr /*: public DisplayMgr*/ {
private:
    TtyScreenBuffer screen_buffer;

    void updateMiniMapSize(const double map_proportion);

    void renderPixelColumn(const uint16_t screen_x, const FovRay& ray);

// TBD: will be inherited from DisplayMgr
protected:
    uint16_t minimap_w;
    uint16_t minimap_h;

public:
    std::string tty_name;

    void initialize(const Settings& settings);

    void fitToWindow(const double map_proportion);

    inline uint16_t screenWidth() { return screen_buffer.w; }

    void clearDisplay();

    void renderView(const std::vector<FovRay>& fov_rays);

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

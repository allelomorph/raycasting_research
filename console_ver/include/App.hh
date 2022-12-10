#ifndef APP_HH
#define APP_HH

#include "State.hh"
#include "FpsCalc.hh"
#include "LinuxKbdInputMgr.hh"
#include "SdlKbdInputMgr.hh"

#include <csignal>               // sig_atomic_t
#include <cstdint>               // uint16_t
#include <cassert>               // uint16_t

#include <string>
#include <vector>


extern volatile std::sig_atomic_t sigint_sigterm_received;
extern volatile std::sig_atomic_t sigwinch_received;

// TBD: should RGBA support be added? Will determine once SDL textures are implemented
struct RgbColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class ASCIIScreenBuffer : public std::string {
public:
    uint16_t w;
    uint16_t h;

    void resizeToDims(const uint16_t _w, const uint16_t _h) {
        w = _w;
        h = _h;
        resize(w * h);
    }

    void resizeToDims(const uint16_t _w, const uint16_t _h, const char c) {
        w = _w;
        h = _h;
        resize(w * h, c);
    }
/*
    char* ptrToCoords(const uint16_t col_i, const uint16_t row_i) {
        assert(col_i < w && row_i < h);
        return data() + ((row_i * w) + col_i);
    }
*/
    char& charAtCoords(const uint16_t col_i, const uint16_t row_i) {
        assert(col_i < w && row_i < h);
        return (*this)[(row_i * w) + col_i];
    }

    void replaceAtCoords(const uint16_t col_i, const uint16_t row_i,
                         const uint16_t len, const char* s) {
        assert(col_i < w && row_i < h);
        replace((row_i * w) + col_i, len, s);
    }

    std::string row(const uint16_t row_i) {
        assert(row_i < h);
        return substr(row_i * w, w);
    }
};

class App {
public:
    App() = delete;
    App(const char* exec_filename, const char* map_filename);
    // TBD: rule of 5?
    ~App();

    void run();
private:
    // TBD: no longer singleton pattern, change from pointer after delegating
    //   tasks in initialize()
    State<LinuxKbdInputMgr>* state;

    std::string exec_filename;
    // TBD: could be member of Layout
    std::string map_filename;

    ProcessTimeFpsCalc pt_fps_calc;
    RealTimeFpsCalc    rt_fps_calc;

    // TBD: establish minimum viable display dimensions in chars (original was 1080 x 640px in SDL)

    // TBD: initially only supporting ASCII art
    //std::vector<RgbColor> screen_buffer;
    //std::string screen_buffer;
    ASCIIScreenBuffer screen_buffer;

    void initialize();

    void getEvents();
    // TBD: move to State::updateFromInput(frame_duration_mvg_avg)?
    void updateData();

    void renderView();
    void renderMap();
    void renderHUD();

    // TBD: drawFrame instead?
    void drawScreen();
};

#endif  // APP_HH

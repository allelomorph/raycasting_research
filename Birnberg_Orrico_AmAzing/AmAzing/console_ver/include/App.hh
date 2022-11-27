#ifndef APP_HH
#define APP_HH

#include "State.hh"
#include "FpsCalc.hh"

#include <csignal>           // sig_atomic_t
#include <cstdint>           // uint16_t

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

class App {
public:
    App() = delete;
    App(const char* exec_filename, const char* map_filename);
    // TBD: rule of 5?
    ~App();

    void run();
private:
    State *state;

    std::string exec_filename;
    std::string map_filename;

    ProcessTimeFpsCalc pt_fps_calc;
    RealTimeFpsCalc rt_fps_calc;

    // TBD: establish minimum viable display dimensions in chars (original was 1080 x 640px in SDL)
    // tty display dimensions in characters
    uint16_t tty_window_w;
    uint16_t tty_window_h;

    // TBD: initially only supporting 3-byte TrueColor
    std::vector<RgbColor> screen_buffer;

    void initialize();
    void getEvents();
    void updateData();
    void printDebugHUD();
};

#endif  // APP_HH

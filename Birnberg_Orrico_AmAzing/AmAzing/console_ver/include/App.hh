#ifndef APP_HH
#define APP_HH

#include "State.hh"
#include "Matrix.hh"  // Vector2d Vector2i

#include <time.h>     // clock_t
#include <termios.h>  // winsize

#include <csignal>    // sig_atomic_t

#include <string>


extern volatile std::sig_atomic_t sigint_sigterm_received;
extern volatile std::sig_atomic_t sigwinch_received;

class FpsCalculator {
private:
    // TBD: udpate to C++ idiomatic time calculation?
    clock_t prev_timepoint;
    clock_t curr_timepoint;
public:
    double moving_avg_frame_time { 0.015 };
    void initialize();
    void calculate();
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

    FpsCalculator fps_calc;

    // TBD: establish minimum viable display dimensions in chars (original was 1080 x 640px in SDL)
    struct winsize winsz;

    void initialize();
    void getEvents();
    void updateData();
    void printDebugHUD();
};

#endif  // APP_HH

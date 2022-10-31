#ifndef APP_HH
#define APP_HH

#include "State.hh"
//#include "../include/Matrix.hh"  // Vector2d Vector2i

#include <time.h>  // clock_t

#include <csignal>        // sig_atomic_t

#include <string>

// console chars, not pixels
// #define WINDOW_WIDTH 108
// #define WINDOW_HEIGHT 64

#define CSI_CURSOR_UP( x ) "\x1b[" #x "A"

extern volatile std::sig_atomic_t stop;

class App {
public:
    App();
    App(const char* exec_filename, const char* map_filename);
    ~App();

    void run();
private:
    State *state;
    std::string exec_filename;
    std::string map_filename;
    // TBD: encapsulate into FPScalculator class and make member of state?
    // TBD: udpate to C++ idiomatic time calculation?
    clock_t prev_timepoint { clock() };
    clock_t curr_timepoint;
    double moving_avg_frame_time { 0.015 };

    void initialize();
    void calculateFPS();
    void getEvents();
    void updateData();
    void printDebugHUD();
};

#endif  // APP_HH

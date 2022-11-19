#ifndef APP_HH
#define APP_HH

#include "State.hh"
#include "FpsCalc.hh"

#include <termios.h>         // winsize

#include <csignal>           // sig_atomic_t

#include <string>


extern volatile std::sig_atomic_t sigint_sigterm_received;
extern volatile std::sig_atomic_t sigwinch_received;

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
    struct winsize winsz;

    void initialize();
    void getEvents();
    void updateData();
    void printDebugHUD();
};

#endif  // APP_HH

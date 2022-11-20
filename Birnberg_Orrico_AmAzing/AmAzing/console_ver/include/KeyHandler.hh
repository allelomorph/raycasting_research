#ifndef KEYHANDLER_HH
#define KEYHANDLER_HH

#include "InputKey.hh"

#include <ctime>           // timeval
#include <linux/input.h>   // input_event

#include <unordered_map>
#include <string>


constexpr int UNINITIALIZED_FD  { -1 };

class KeyHandler {
public:
    std::string input_tty_name;                  // TBD: only public for debug?
private:
    std::string display_tty_name;                // TBD: move this to State if display is encapsulated separately?
    std::string kbd_device_path;
    int kbd_device_fd { UNINITIALIZED_FD };      // need fd to pass to ioctl()
    fd_set rdfds;                                // for use by select()
    // When in tty mode, select blocks getKeyEvents and thus the main game loop
    //   until the keyboard device file represented by kbd_device_fd fills with
    //   input_events to read. This means that the timeout for select
    //   effectively creates a real time FPS cap.
    struct timeval select_timeout { 0, 10000 };  // .01 sec / 100 RT FPS cap
    struct input_event ev[32];                   // structs read from kbd_device_fd

    void ungrabDevice();
    void grabDevice(std::string& exec_filename);
public:
    // TBD: make private unless debugging?
    std::unordered_map<LinuxKeyCode, InputKey> key_states;

    KeyHandler();
    ~KeyHandler();

    bool isPressed(LinuxKeyCode keysym);
    bool isReleased(LinuxKeyCode keysym);

    void initialize(std::string& exec_filename);
    void getKeyEvents();
    // void handleKeyEvent(const struct input_event& ev);
};

#endif  // KEYHANDLER_HH

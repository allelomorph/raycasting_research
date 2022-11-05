#ifndef KEYHANDLER_HH
#define KEYHANDLER_HH

#include "InputKey.hh"

#include <linux/input.h>   // input_event

#include <unordered_map>
// #include <set>
#include <string>


constexpr int UNINITIALIZED_FD  { -1 };

class KeyHandler {
private:
    std::string input_tty_name;
    std::string display_tty_name;            // TBD: move this to State if display is encapsulated separately?
    std::string kbd_device_path;
    int kbd_device_fd { UNINITIALIZED_FD };  // need fd to pass to ioctl()
    fd_set rdfds;                            // for use by select()
    struct input_event ev[64];               // structs read from kbd_device_fd

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

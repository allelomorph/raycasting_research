#ifndef LINUXKBDINPUTMGR_HH
#define LINUXKBDINPUTMGR_HH


#include "KeyState.hh"

#include <cstdint>         // uint16_t
#include <ctime>           // timeval
#include <linux/input.h>   // input_event

#include <unordered_map>
#include <string>


class LinuxKbdInputMgr {
private:
    std::unordered_map<LinuxKeyCode, LinuxKeyState> key_states;

    std::string input_tty_name;
    std::string kbd_device_path;

    static constexpr int UNINITIALIZED_FD  { -1 };
    int kbd_device_fd { UNINITIALIZED_FD };      // need fd to pass to ioctl()

    // TBD: should these be variables in case of re-init?
    static constexpr char INPUT_EVENT_PATH_PREFIX[] {
        "/dev/input/event" };                    // used by determineInputDevice
    static constexpr char INPUT_DEVICES_PATH[] {
        "/proc/bus/input/devices" };             // used by determineInputDevice

    fd_set rdfds;                                // used by select() in consumeKeyEvents
    // When in tty mode, select() blocks consumeKeyEvents and thus the main game
    //   loop until the keyboard device file represented by kbd_device_fd fills
    //   with input_event structs to read. This means that the timeout for
    //   select() effectively creates a real time FPS cap.
    struct timeval select_timeout { 0, 10000 };  // .01 sec, or 100 RTFPS cap

    struct input_event ev[32];                   // events read from kbd_device_fd

    // take exclusive access to Linux keyboard events
    void grabDevice(const std::string& exec_filename);
    // return access to keyboard events
    void ungrabDevice();
    // parse and rate entries in /proc/bus/input/devices to find keyboard device file
    std::string determineInputDevice();
    // find a true non-pty tty to provide keyboard input focus
    std::string determineInputTty();

public:
    LinuxKbdInputMgr();
    ~LinuxKbdInputMgr();

    void initialize(const std::string& exec_filename);

    void consumeKeyEvents();

    bool keyDownThisFrame(const LinuxKeyCode keysym);
    bool isPressed(const LinuxKeyCode keysym);
    bool isReleased(const LinuxKeyCode keysym);
    // Game frames may pass between consuming a key press event and its first
    //   autorepeat event; so to maintain the distinction between a key being
    //   pressed and a key being held down at frame granularity, this can be
    //   used to mark any keys pressed this frame as held (repeat) at the end
    //   of that frame
    void decayToAutorepeat();
};


#endif  // LINUXKBDINPUTMGR_HH

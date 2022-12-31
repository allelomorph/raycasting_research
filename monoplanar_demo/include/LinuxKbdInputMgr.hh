#ifndef LINUXKBDINPUTMGR_HH
#define LINUXKBDINPUTMGR_HH

#include "KbdInputMgr.hh"

#include <sys/select.h>    // fd_set (timeval via time_t.h)
#include <linux/input.h>   // input_event

#include <string>


class LinuxKbdInputMgr : public KbdInputMgr {
private:
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
    LinuxKbdInputMgr() = delete;
    LinuxKbdInputMgr(const std::string& exec_filename);
    ~LinuxKbdInputMgr();

    // See notes on consume key functions in parent class
    void consumeKeyEvents();
};


#endif  // LINUXKBDINPUTMGR_HH

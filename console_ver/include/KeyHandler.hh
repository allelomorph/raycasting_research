#ifndef KEYHANDLER_HH
#define KEYHANDLER_HH

#include <cstdint>         // uint16_t
#include <ctime>           // timeval
#include <linux/input.h>   // input_event

#include <unordered_map>
#include <string>


// input_event.code is __u16 in /linux/input.h, should be KEY_*, see /linux/input-event-codes.h
using LinuxKeyCode = uint16_t;
// input_event.value is __s32 in /linux/input.h
enum class LinuxKeyValue { Release, Press, Autorepeat };


class KeyState {
private:
    bool pressed { false };  // key is down
    bool repeat  { false };  // key down event was in earlier frame

public:
    LinuxKeyCode code;

    // KeyState() only needed if map operator[] called with missing key
    KeyState() = delete;
    KeyState(const LinuxKeyCode c) : code(c) {};

    inline void update(const LinuxKeyValue value) {
        if (value < LinuxKeyValue::Autorepeat) {
            repeat = false;
            pressed = static_cast<bool>(value);
        }
    }

    inline bool keyDownThisFrame() {
        return (pressed && !repeat);
    }

    inline bool isPressed() {
        return (pressed);
    }

    inline bool isReleased() {
        return (!pressed);
    }

    inline void decayToAutorepeat() {
        if (pressed)
            repeat = true;
    }
};

constexpr int UNINITIALIZED_FD  { -1 };

class KeyHandler {
private:
    std::unordered_map<LinuxKeyCode, KeyState> key_states;

    // TBD: move tty names to State if display is encapsulated separately?
    std::string input_tty_name;
    std::string display_tty_name;

    std::string kbd_device_path;
    int kbd_device_fd { UNINITIALIZED_FD };      // need fd to pass to ioctl()
    fd_set rdfds;                                // for use by select()
    // When in tty mode, select() blocks getKeyEvents and thus the main game
    //   loop until the keyboard device file represented by kbd_device_fd fills
    //   with input_event structs to read. This means that the timeout for
    //   select() effectively creates a real time FPS cap.
    struct timeval select_timeout { 0, 10000 };  // .01 sec, or 100 RTFPS cap
    struct input_event ev[32];                   // read from kbd_device_fd

    void ungrabDevice();
    void grabDevice(const std::string& exec_filename);
public:
    KeyHandler();
    ~KeyHandler();

    bool keyDownThisFrame(const LinuxKeyCode keysym);
    bool isPressed(const LinuxKeyCode keysym);
    bool isReleased(const LinuxKeyCode keysym);

    void initialize(const std::string& exec_filename);
    void getKeyEvents();
    void decayToAutorepeat();
};

#endif  // KEYHANDLER_HH

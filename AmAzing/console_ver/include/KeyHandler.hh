#ifndef KEYHANDLER_HH
#define KEYHANDLER_HH


#include <linux/input.h>   // input_event
#include <cstdint>         // uint16_t uint32_t

#include <unordered_map>
#include <set>
#include <string>

#include <iostream>  // TBD debug

// input_event.code is __u16 in /linux/input.h
using LinuxKeyCode = uint16_t;
// input_event.value is __u32 in /linux/input.h
using LinuxKeyValue = uint32_t;

constexpr LinuxKeyValue VAL_RELEASE    { 0 };
constexpr LinuxKeyValue VAL_PRESS      { 1 };
constexpr LinuxKeyValue VAL_AUTOREPEAT { 2 };

// Original design intent was for InputKey to be an interface class for children
//   LiveKey, ToggleOnOffKey, and ToggleSelectionKey, which would each have
//   their own versions of updateState. But this prevented having a single
//   key_states map, as std::unordered_map cannot be instantiated with an
//   abstract class.
//
// Also found that it is possible to use SFINAE and std::enable_if with member
//   functions, see: https://stackoverflow.com/a/50561421,
//   although passing an unresolved template to unordered_map<> could cause the same problem.
enum class KeyType { Live, ToggleOnOff, /*ToggleSelector*/ };

class InputKey {
private:
    KeyType type;
    bool pressed { false };
    // std::set<InputKey*>* toggle_selections;
public:
    LinuxKeyCode code;
    std::string repr;

    // Default (void param) constructor needed for std::map/set (eg.
    //   operator[] with a missing key.
/*
    InputKey() {
        throw std::runtime_error(
            "InputKey() should never be called, as key_states should never be "
            "directly accessed with an unknown key");
    };
*/
    InputKey(KeyType kt, LinuxKeyCode c, const char* r) :
        type(kt), code(c), repr(r) {};
    /*
    InputKey(KeyType kt, LinuxKeyCode c, const char* r, std::set<InputKey*>& s) :
        type(kt), code(c), repr(r) {
        if (kt == KeyType::ToggleSelector) {
            toggle_selections = &s;
            toggle_selections->emplace(this);
        }
    };
    */

    void updateState(LinuxKeyValue value) {
        if (type == KeyType::Live) {
            if (value < VAL_AUTOREPEAT) {
                pressed = value;
            }
        } else if (type == KeyType::ToggleOnOff) {
            if (value == VAL_PRESS) {
                pressed = !pressed;
            }
        } /* else if (type == KeyType::ToggleSelector) {
            if (value == VAL_PRESS) {
                for (auto key : (*toggle_selections)) {
                    key->pressed = false;
                }
                pressed = true;
            }
        }
          */
    }

    inline bool operator<(const InputKey& rhs) const {
        return code < rhs.code;
    }
    inline bool isPressed() {
        return (pressed == true);
    }
    inline bool isReleased() {
        return (pressed == false);
    }
};


class KeyHandler {
private:
    std::string input_tty_name;
    std::string display_tty_name;
    std::string kbd_device_path;
    int kbd_fd;
    struct input_event ev[64];
    fd_set rdfds;

public:
    std::unordered_map<LinuxKeyCode, InputKey> key_states;
    KeyHandler();

    bool isPressed(LinuxKeyCode keysym);
    bool isReleased(LinuxKeyCode keysym);
    void getKeyEvents();
    void handleKeyEvent(const struct input_event& ev);
    void initialize(std::string exec_filename);
};

#endif  // KEYHANDLER_HH

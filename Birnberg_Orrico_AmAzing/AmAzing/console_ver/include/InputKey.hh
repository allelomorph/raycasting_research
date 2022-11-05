#ifndef INPUTKEY_HH
#define INPUTKEY_HH

#include <cstdint>         // uint16_t uint32_t

// #include <set>
#include <string>

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
//   although passing an unresolved template to unordered_map<> could cause
//   the same problem.
//
// TBD: InputKey() = delete is valid if we never use map<>::operator[] with a
//   novel key. Does this mean we can switch to the
//   template<KeyType KT>InputKey(...) strategy?

enum class KeyType { Live, ToggleOnOff, /*ToggleSelector*/ };

class InputKey {
private:
    KeyType type;
    bool pressed { false };
    // std::set<InputKey*>* toggle_selections;
public:
    LinuxKeyCode code;
    std::string repr;

    // InputKey() only needed if map operator[] called with missing key
    InputKey() = delete;
    InputKey(KeyType kt, LinuxKeyCode c, const char* r) :
        type(kt), code(c), repr(r) {};
    // InputKey(KeyType kt, LinuxKeyCode c, const char* r, std::set<InputKey*>& s);

    void updateState(LinuxKeyValue value);
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


#endif  // INPUTKEY_HH

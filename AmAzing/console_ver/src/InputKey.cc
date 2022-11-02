#include "InputKey.hh"

#include <type_traits>   // enable_if


/*
InputKey::InputKey(KeyType kt, LinuxKeyCode c, const char* r,
                   std::set<InputKey*>& s) : type(kt), code(c), repr(r) {
    if (kt == KeyType::ToggleSelector) {
        toggle_selections = &s;
        toggle_selections->emplace(this);
    }
}
*/

void InputKey::updateState(LinuxKeyValue value) {
    if (type == KeyType::Live) {
        if (value < VAL_AUTOREPEAT) {
            pressed = value;
        }
    } else if (type == KeyType::ToggleOnOff) {
        if (value == VAL_PRESS) {
            pressed = !pressed;
        }
    }
}

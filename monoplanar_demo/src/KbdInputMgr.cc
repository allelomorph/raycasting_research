#include "KbdInputMgr.hh"


bool KbdInputMgr::keyDownThisFrame(const int32_t keysym) const {
    // operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.keyDownThisFrame());
    return false;
}

bool KbdInputMgr::isPressed(const int32_t keysym) const {
    // operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.isPressed());
    return false;
}

bool KbdInputMgr::isReleased(const int32_t keysym) const {
    // operator[] inserts a new key if not found
    auto ks_it { key_states.find(keysym) };
    if (ks_it != key_states.end())
        return (ks_it->second.isReleased());
    return true;
}

void KbdInputMgr::decayToAutorepeat() {
    for (auto& pair : key_states) {
        pair.second.decayToAutorepeat();
    }
}

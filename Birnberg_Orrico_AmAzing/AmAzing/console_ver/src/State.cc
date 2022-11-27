#include "State.hh"


// singleton class pattern
State *State::instance = new State;

// singleton class pattern
State *State::getInstance() {
    return instance;
}

State::~State() {
    if (layout != nullptr)
        delete layout;
}

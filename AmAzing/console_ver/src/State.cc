#include <iomanip>
#include <iostream>
#include <cstdint>   // uint16_t

#include "State.hh"

State *State::instance = new State;

State *State::getInstance() {
    return instance;
}

State::~State() {
    if (layout != nullptr)
        delete layout;
}

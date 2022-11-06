#include "State.hh"


State *State::instance = new State;

State *State::getInstance() {
    return instance;
}

State::~State() {
    if (layout != nullptr)
        delete layout;
}
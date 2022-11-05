#ifndef STATE_HH
#define STATE_HH

#include "KeyHandler.hh"
#include "Layout.hh"
//#include "../include/Matrix.hh"  // Vector2d

class State {
private:
    // TBD: why a pointer here? To delay initialization from State intstantiation until call of App::initialize()?
    static State *instance;

public:
    ~State();

    bool done  { false };
    bool showFPS  { false };
    bool showMap  { false };

    KeyHandler key_handler;
    Layout *layout { nullptr };

    static State *getInstance();
};

#endif  // STATE_HH
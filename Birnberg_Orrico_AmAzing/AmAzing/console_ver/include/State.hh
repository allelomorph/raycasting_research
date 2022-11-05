#ifndef STATE_HH
#define STATE_HH

#include "Layout.hh"
#include "KeyHandler.hh"
#include "Matrix.hh"  // Vector2d


class State {
private:
    // TBD: why a pointer here? To delay initialization from State intstantiation until call of App::initialize()?
    //    Likely to allow reinit of state, and static so that all instances of App share the same state...?
    //    Pointer instead of reference due to reference members not able to be initialized without something to point to.
    // self-referential, needs forward declaration
    static State *instance;

public:
    // TBD: rule of 5?
    ~State();

    static State *getInstance();

    // operation flags
    bool done  { false };
    bool showFPS  { false };
    bool showMap  { false };

    // map
    Layout *layout { nullptr };

    // input
    KeyHandler key_handler;

    // raycasting
    Vector2d pos;
    Vector2d dir;
    Vector2d viewPlane;
};

#endif  // STATE_HH

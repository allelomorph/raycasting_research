#ifndef STATE_HH
#define STATE_HH

#include <vector>
#include <unordered_map>

#include "KeyHandler.hh"
#include "Layout.hh"
//#include "../include/Matrix.hh"  // Vector2d

// used Eigen::Vector2d

class State {
private:
    // TBD: why a static pointer to its own class? to ensure that only one can
    //   be referenced no matter how many times the constructor is called?
    static State *instance;
public:
    ~State();
    bool done = false;
    bool showFPS = false;
    bool showMap = false;
    //Vector2d pos;
    //Vector2d dir;
    //Vector2d viewPlane;
    // TBD: where is u_char defined?
    // TBD: why are we mapping textures to chars? and why unordered?
    KeyHandler key_handler;
    Layout *layout { nullptr };
    static State *getInstance();
};

#endif  // STATE_HH

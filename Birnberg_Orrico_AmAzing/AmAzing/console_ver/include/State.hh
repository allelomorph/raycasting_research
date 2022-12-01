#ifndef STATE_HH
#define STATE_HH

#include "Layout.hh"
#include "KeyHandler.hh"
#include "Matrix.hh"  // Vector2d


// TBD: is a singleton pattern truly necessary for State? All we need to assure
//   is that each instance of App has only one instance of State; may be better
//   to simply deactivate copy/move ops with `= delete`
class State {
private:
    // singleton class pattern
    static State *instance;

public:
    // operation flags
    bool done       { false };

    /*
     * game settings
     */
    // TBD: on by default during testing
    bool   show_map       { true };
    // length of map display square side as percentage of sceen height
    double map_proportion { 1.0/3 };
    uint16_t map_h;
    uint16_t map_w;
    bool   show_fps       { false };
    // TBD: on by default during development
    bool debug_mode       { true };

    // map
    Layout* layout { nullptr };

    // user input
    KeyHandler key_handler;

    /*
     * raycasting
     */
    // position vector (player x and y coordinates on map grid)
    Vector2d player_pos;
    // direction vector (represented as line segment on map grid from player
    //   position to midpoint of view plane)
    Vector2d player_dir;
    // The view/camera plane is the "window" through which the player sees the
    //   world, and in the map grid, is represented as a line segment that is
    //   perpendicular to and bisected by the direction vector.
    // Using vector operations, the camera plane could be said to run from
    //   (pos + dir - view_plane) on the player's left, intersect with dir at
    //   (pos + dir), and end at the player's right with (pos + dir + view_plane)
    // The ratio of the lengths of the direction vector and the camera plane
    //   determines the FOV angle, so unless the desire is to change the FOV,
    //   they must change together proportionally. FOV can be calculated with
    //   2 * atan(view_plane length/dir length).
    // When the player rotates, the values of dir and plane will be changed,
    //   but should always remain perpendicular and of constant length.
    Vector2d view_plane;

    // used to determine player movement speed, as pegged to frame rate
    double base_movement_rate;
    // expressed as percentage of base_movement_rate
    double turn_rate;

    // TBD: rule of 5?
    ~State();

    // singleton class pattern
    static State *getInstance();
};

#endif  // STATE_HH

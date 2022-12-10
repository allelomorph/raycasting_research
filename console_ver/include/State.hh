#ifndef STATE_HH
#define STATE_HH

#include "Layout.hh"
#include "LinuxKbdInputMgr.hh"
#include "SdlKbdInputMgr.hh"
#include "Matrix.hh"             // Vector2d


template <typename KbdInputMgrType>
class State {
    static_assert(std::is_same<KbdInputMgrType, LinuxKbdInputMgr>::value ||
                  std::is_same<KbdInputMgrType, SdlKbdInputMgr>::value,
                  "Incompatible type for State::kbd_input_mgr");

public:
    // Each instance of State should only be attached to an instance of App, so
    //   we prohibit the default definitions of its copy and move operations:
    // TBD: uncomment pending changes to App::initialize()
    //State(const State&)             = delete;
    //State& operator=(const State&)  = delete;
    //State(const State&&)            = delete;
    //State& operator=(const State&&) = delete;

    /*
     * operation flags
     */
    //IoMode io_mode (::TTY or ::SDL)
    bool stop   { false };
    //bool pause  { false };

    /*
     * game settings
     */
    // TBD: reset defaults after development
    bool     show_map       { true };
    // map HUD hieght : screen height
    double   map_proportion { 1.0/3 };
    uint16_t map_h;
    uint16_t map_w;
    bool     show_fps       { false };
    bool     debug_mode     { true };
    // when true, use real ray distance to wall rather than perpendicular
    //   camera plane distance
    bool     fisheye        { false };

    /*
     * floor map (with starting positions of actors)
     */
    // using pointer to control when constructor is called: during init
    //   TBD: or when new map file is loaded
    Layout* layout { nullptr };

    /*
     * user input
     */
    KbdInputMgrType kbd_input_mgr;

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

    ~State() {
        if (layout != nullptr)
            delete layout;
    }
};

#endif  // STATE_HH

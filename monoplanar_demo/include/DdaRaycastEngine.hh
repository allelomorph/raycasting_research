#ifndef DDARAYCASTENGINE_HH
#define DDARAYCASTENGINE_HH

#include "Matrix.hh"    // Vector2d
#include "Layout.hh"
#include "Settings.hh"

#include <cstdint>

#include <vector>


enum class WallOrientation { EW, NS };

struct FovRay {
    Vector2d        dir;            // ray direction
    double          wall_dist;      // distance to first wall collision
    WallOrientation type_wall_hit;  // EW or NS alignment of wall hit
    // TBD: SDL_Texture* wall_texture, or
    // TBD: uint16_t map_x, map_y
};

/*
struct FovRay {
private:
    struct WallHit {
        double          dist    { 0.0 };  // distance to first wall collision
        WallOrientation algnmt;           // EW or NS alignment of wall hit
        uint16_t        map_x   { 0 };    // wall map tile x
        uint16_t        map_y   { 0 };    // wall map tile y
    };
public:
    Vector2d            dir;              // ray direction
    WallHit             wall_hit;
};
*/

class DdaRaycastEngine {
public:
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

    uint16_t screen_w;

    // std::vector<FovRay> fov_rays;

    // float fov_angle;

    // TBD: store layout here? (updateData still needs access for movement)

    DdaRaycastEngine();

    void updateScreenSize(const uint16_t w);

    FovRay castRay(const uint16_t screen_x, const Layout& layout,
                   const Settings& settings);

    // void castRays(const Settings& settings);
};


#endif  // DDARAYCASTENGINE_HH

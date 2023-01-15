#ifndef DDARAYCASTENGINE_HH
#define DDARAYCASTENGINE_HH

#include "Matrix.hh"    // Vector2d
#include "Layout.hh"
#include "Settings.hh"

#include <cstdint>

#include <vector>


enum class WallOrientation { NS, EW };

struct FovRay {
private:
    struct WallHit {
        double          dist { 0.0 };     // distance to first wall collision
        WallOrientation algnmt;           // NS or EW alignment of wall hit
        uint8_t         tex_key;          // layout.tile(map_x, map_y)
        double          x;                // when viewing grid tile as wall unit
                                          //   from player's perspective, x in
                                          //   wall unit face where ray hit
                                          //   (expressed as fraction of wall unit,
                                          //   with 0.0 to player's left when
                                          //   facing side hit)
    };
public:
    Vector2d            dir;              // ray direction
    WallHit             wall_hit;
};

class DdaRaycastEngine {
private:
    // TBD: better as member of Matrix?
    // rotate Vector counterclockwise around origin
    Vector2d rotateVector2d(const Vector2d& vec, const double radians);

    // used to set FOV from window aspect ratio to maintain square wall units
    //   in rendered view (calibrated from reference (627w:480h):0.666666 =
    //   1.959375, rounded up as calibration was by eye)
    static constexpr double ASPECT_RATIO_TO_VIEW_PLANE_MAG_RATIO { 2 };
    // when in tty mode, the "pixels," or monospace terminal characters, are not
    //   square, but 1w:2h, so we compensate with the inverse
    static constexpr double CHAR_PX_ASPECT_RATIO_TO_VIEW_PLANE_MAG_RATIO {
        ASPECT_RATIO_TO_VIEW_PLANE_MAG_RATIO * 2 };

public:
    // position vector (player x and y coordinates on map grid)
    Vector2d player_pos;
    // direction vector (represented as line segment on map grid from player
    //   position to midpoint of view plane; always unit vector; begins facing north)
    Vector2d player_dir { 0, 1 };
    // The view/camera plane is the "window" through which the player sees the
    //   world, and in the map grid, is represented as a line segment that is
    //   perpendicular to and bisected by the direction vector.
    // Using vector operations, the camera plane could be said to run from
    //   (pos + dir - view_plane) on the player's left, intersect with dir at
    //   (pos + dir), and end at the player's right with (pos + dir + view_plane)
    // The ratio of magnitudes of the direction vector and the camera plane
    //   determines the FOV angle, calculated with 2 * atan(magnitude of
    //   view_plane) / magnitude of player_dir, or just 2 * atan(magnitude of
    //   view_plane), as player_dir should always be a unit vector.
    // When the player rotates, player_dir and view_plane should always remain
    //   perpendicular and of constant magnitude.
    Vector2d view_plane { 1, 0 };

    uint16_t screen_w;
    std::vector<FovRay> fov_rays;

    Layout layout;

    void fitToWindow(const bool tty_io,
                     const uint16_t w, const uint16_t h);

    inline void loadMapFile(const std::string& map_filename) {
        layout.loadMapFile(map_filename, player_pos);
    }

    void castRay(const uint16_t screen_x, const Settings& settings);
    void castRays(const Settings& settings);

    void playerTurnLeft(const double rot_speed);
    void playerTurnRight(const double rot_speed);
    void playerStrafeLeft(const double move_speed);
    void playerStrafeRight(const double move_speed);
    void playerMoveFwd(const double move_speed);
    void playerMoveBack(const double move_speed);
};


#endif  // DDARAYCASTENGINE_HH

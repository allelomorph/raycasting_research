#ifndef DDARAYCASTENGINE_HH
#define DDARAYCASTENGINE_HH

#include "Vector2d.hh"
#include "Layout.hh"
#include "Settings.hh"

#include <cstdint>

#include <vector>


enum class WallOrientation { NS, EW };

struct FovRay {
    // ray direction
    Vector2d dir;
    struct WallHit {
        // distance to first wall collision
        double          dist { 0.0 };
        // NS or EW alignment of wall hit
        WallOrientation algnmt;
        // layout.tile(map_x, map_y)
        uint8_t         tex_key;
        // when viewing a grid tile as a square wall unit from the player's
        //   perspective, x in the wall unit face where ray hit (expressed as
        //   fraction of wall unit, with 0.0 to player's left when facing wall)
        double          x;
    }        wall_hit;
};

class DdaRaycastEngine {
private:
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
    // window horizontal pixel count
    uint16_t window_w;

    std::vector<FovRay> fov_rays;

    Layout layout;

    /**
     * @brief scale engine to window size
     *
     * @param tty_io - game currently in tty display mode
     * @param w      - window width in pixels
     * @param h      - window height in pixels
     */
    void fitToWindow(const bool tty_io,
                     const uint16_t w, const uint16_t h);

    /**
     * @brief parse map file into stage layout
     *
     * @param map_filename - map file
     */
    inline void loadMapFile(const std::string& map_filename) {
        layout.loadMapFile(map_filename, player_pos);
    }

    /**
     * @brief apply DDA algorithm to cast ray from player position to first wall
     *   hit
     *
     * @param window_x - horizontal window pixel coordinate
     * @param settings - current game settings
     */
    void castRay(const uint16_t window_x, const Settings& settings);
    /**
     * @brief cast all rays in FOV
     *
     * @param settings - current game settings
     */
    void castRays(const Settings& settings);

    /**
     * @brief rotate player counterclockwise
     *
     * @param rot_speed - player rotation speed, pegged to FPS
     */
    void playerTurnLeft(const double rot_speed);
    /**
     * @brief rotate player clockwise
     *
     * @param rot_speed - player rotation speed, pegged to FPS
     */
    void playerTurnRight(const double rot_speed);
    /**
     * @brief move player to left
     *
     * @param move_speed - travel speed across map, pegged to FPS
     */
    void playerStrafeLeft(const double move_speed);
    /**
     * @brief move player to right
     *
     * @param move_speed - travel speed across map, pegged to FPS
     */
    void playerStrafeRight(const double move_speed);
    /**
     * @brief move player forward
     *
     * @param move_speed - travel speed across map, pegged to FPS
     */
    void playerMoveFwd(const double move_speed);
    /**
     * @brief move player backward
     *
     * @param move_speed - travel speed across map, pegged to FPS
     */
    void playerMoveBack(const double move_speed);
};


#endif  // DDARAYCASTENGINE_HH

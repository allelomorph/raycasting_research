#include "DdaRaycastEngine.hh"

#include <cstdint>
#include <cmath>     // cos, sin, sqrt


void DdaRaycastEngine::fitToWindow(const bool tty_io,
                                   const uint16_t w, const uint16_t h) {
    screen_w = w;
    fov_rays.resize(screen_w);

    // widen FOV to match aspect ratio to always render square-looking wall units
    double curr_aspect_ratio { double(w) / h };
    double target_view_plane_mag { curr_aspect_ratio /
                                   (tty_io ?
                                    CHAR_PX_ASPECT_RATIO_TO_VIEW_PLANE_MAG_RATIO :
                                    ASPECT_RATIO_TO_VIEW_PLANE_MAG_RATIO) };
    double curr_view_plane_mag { std::sqrt(
            (view_plane.x * view_plane.x) +
            (view_plane.y * view_plane.y) ) };
    double target_vpm_to_curr_vpm_ratio { target_view_plane_mag / curr_view_plane_mag };
    view_plane.x *= target_vpm_to_curr_vpm_ratio;
    view_plane.y *= target_vpm_to_curr_vpm_ratio;
}

void DdaRaycastEngine::castRay(const uint16_t screen_x,
                               const Settings& settings) {
    // x coordinate in the camera plane represented by the current
    //   screen x coordinate, calculated so that the left edge of the
    //   camera plane is -1.0, center 0.0, and right edge is 1.0
    double camera_x { 2 * screen_x / double(screen_w) - 1 };
    FovRay ray;

    // ray origin is player_pos
    // multiply camera_plane vector by scalar x, then add to direction vector
    //   to get ray direction
    ray.dir = player_dir + (view_plane * camera_x);

    // current map grid coordinates of ray
    uint16_t map_x ( player_pos.x );
    uint16_t map_y ( player_pos.y );

    // Initially the distances from the ray origin (player position) to its
    //   first intersections with a map unit grid vertical and horizonal,
    //   respectively. These map grid lines, or integer values of x and y,
    //   serve to represent wall boundaries when they border a grid square
    //   designated as a wall.
    double dist_next_unit_x;
    double dist_next_unit_y;

    // distances the ray has to travel to go from one unit grid vertical
    //   to the next, or one horizontal to the next, respectively
    // IEEE 754 floating point values in C++ protect against division by 0
    double dist_per_unit_x { std::abs(1 / ray.dir.x) };
    double dist_per_unit_y { std::abs(1 / ray.dir.y) };

    // DDA algorithm will always jump exactly one map grid square each
    //   loop, either in the x or y. These vars record those increments,
    //   either -1 or +1
    int8_t map_step_x;
    int8_t map_step_y;

    // setup map grid step and initial ray distance to next grid unit values
    if (ray.dir.x < 0) {
        map_step_x = -1;
        dist_next_unit_x = (player_pos.x - map_x) * dist_per_unit_x;
    } else {
        map_step_x = 1;
        dist_next_unit_x = (map_x + 1.0 - player_pos.x) * dist_per_unit_x;
    }
    if (ray.dir.y < 0) {
        map_step_y = -1;
        dist_next_unit_y = (player_pos.y - map_y) * dist_per_unit_y;
    } else {
        map_step_y = 1;
        dist_next_unit_y = (map_y + 1.0 - player_pos.y) * dist_per_unit_y;
    }

    // perform DDA algo, or the incremental casting of the ray
    // moves to a new map unit square every loop, as directed by map_step values
    // TBD: does orientation need to be set every loop?
    WallOrientation alignment;
    while (!layout.tileIsWall(map_x, map_y)) {
        if (dist_next_unit_x < dist_next_unit_y) {
            dist_next_unit_x += dist_per_unit_x;
            map_x += map_step_x;
            alignment = WallOrientation::NS;
        } else {
            dist_next_unit_y += dist_per_unit_y;
            map_y += map_step_y;
            alignment = WallOrientation::EW;
        }
        // TBD: what about OneLoneCoder's max cast distance?
    }

    ray.wall_hit.algnmt = alignment;
    ray.wall_hit.tex_key = layout.tile(map_x, map_y);

    // Calculate distance to wall hit from camera plane, moving
    //   perpendicular to the camera plane. If the actual length of the
    //   ray cast from player to wall is used, one can observe a projection
    //   that is more distorted as one approaches the wall.

    // TBD: this paragarph is now conjecture: ", the expected result would be
    //   a fisheye effect. For example, if the player were squarely
    //   facing a wall (wall parallel to camera plane,) in the resulting
    //   render the wall should appear of even height. If using the
    //   length of the rays as they hit the wall, the wall would instead
    //   appear to taper to the left and right ends of the display."
    //
    // One way to calculate the perpendicular camera plane distance would be
    //   to use the formula for shortest distance from a point to a line,
    //   where the point is where the wall was hit, and the line is the
    //   camera plane.
    // TBD: evaluate and corroborate this last paragraph
    // However, it can be done more simply: due to how dist_per_unit_? and
    //   dist_next_unit_? were scaled by a factor of |ray.dir| above, the
    //   length of dist_next_unit_? already almost equals the perpendicular
    //   camera plane distance. We just need to subtract dist_per_unit_? once,
    //   going one step back in the casting, as in the DDA loop above we went
    //   one step further to end up inside the wall.

    if (settings.euclidean) {  // actual ray distance from player_pos
        ray.wall_hit.dist = (ray.wall_hit.algnmt == WallOrientation::NS) ?
            dist_next_unit_x : dist_next_unit_y;
    } else {                   // perpendicular distance from camera plane
        ray.wall_hit.dist = (ray.wall_hit.algnmt == WallOrientation::NS) ?
            dist_next_unit_x - dist_per_unit_x :
            dist_next_unit_y - dist_per_unit_y;
    }

    // Translate coordinate vector of ray's wall hit (in map view, from "above")
    //   into x coordinate in wall segment as seen from the perspective of a
    //   player facing the wall.
    ray.wall_hit.x = (ray.wall_hit.algnmt == WallOrientation::EW) ?
            player_pos.x + (ray.wall_hit.dist * ray.dir.x) :
            player_pos.y + (ray.wall_hit.dist * ray.dir.y);
    // Expressed as fraction of 1 grid unit (0.0 on the left)
    ray.wall_hit.x -= std::floor(ray.wall_hit.x);

    fov_rays[screen_x] = ray;
}

void DdaRaycastEngine::castRays(const Settings& settings) {
    for (uint16_t screen_x { 0 }; screen_x < screen_w; ++screen_x) {
        castRay(screen_x, settings);
    }
}

// CCW
void DdaRaycastEngine::playerTurnLeft(const double rot_speed) {
    player_dir.rotate(rot_speed);
    view_plane.rotate(rot_speed);
}

// CW
void DdaRaycastEngine::playerTurnRight(const double rot_speed) {
    player_dir.rotate(-rot_speed);
    view_plane.rotate(-rot_speed);
}

// Vector2d is { x, y }
void DdaRaycastEngine::playerStrafeLeft(const double move_speed) {
    double start_ppx { player_pos.x };
    if (!layout.tileIsWall(player_pos.x - player_dir.y * move_speed, player_pos.y))
        player_pos.x -= player_dir.y * move_speed;
    if (!layout.tileIsWall(start_ppx, player_pos.y + player_dir.x * move_speed))
        player_pos.y += player_dir.x * move_speed;
}

void DdaRaycastEngine::playerStrafeRight(const double move_speed) {
    double start_ppx { player_pos.x };
    if (!layout.tileIsWall(player_pos.x + player_dir.y * move_speed, player_pos.y))
        player_pos.x += player_dir.y * move_speed;
    if (!layout.tileIsWall(start_ppx, player_pos.y - player_dir.x * move_speed))
        player_pos.y -= player_dir.x * move_speed;
}

// Vector2d is { x, y }
void DdaRaycastEngine::playerMoveFwd(const double move_speed) {
    double start_ppx { player_pos.x };
    if (!layout.tileIsWall(player_pos.x + player_dir.x * move_speed, player_pos.y))
        player_pos.x += player_dir.x * move_speed;
    if (!layout.tileIsWall(start_ppx, player_pos.y + player_dir.y * move_speed))
        player_pos.y += player_dir.y * move_speed;
}

void DdaRaycastEngine::playerMoveBack(const double move_speed) {
    double start_ppx { player_pos.x };
    if (!layout.tileIsWall(player_pos.x - player_dir.x * move_speed, player_pos.y))
        player_pos.x -= player_dir.x * move_speed;
    if (!layout.tileIsWall(start_ppx, player_pos.y - player_dir.y * move_speed))
        player_pos.y -= player_dir.y * move_speed;
}

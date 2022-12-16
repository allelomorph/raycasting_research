#include "DdaRaycastEngine.hh"

#include <cstdint>


Vector2d DdaRaycastEngine::rotateVector2d(const Vector2d& vec,
                                          const double radians) {
    /*
      Matrix2d rotate;
      rotate <<
          std::cos(rot_speed), -std::sin(rot_speed),
          std::sin(rot_speed), std::cos(rot_speed);
      return (rotate * vector);
    */
    // TBD: currently using manual transposition of vector rotation formula;
    //   debug the above rotation matrix population order
    return Vector2d {
        std::cos(radians) * vec(0) - std::sin(radians) * vec(1),
        std::sin(radians) * vec(0) + std::cos(radians) * vec(1)
    };
}

DdaRaycastEngine::DdaRaycastEngine() {
    // Starting direction vector is a bit longer than the camera plane, so
    //   the FOV will be smaller than 90° (more precisely, the FOV is
    //   2 * atan(0.66 (magnitude of view_plane) / 1.0 (magnitude of
    //   player_dir)), or 66°.
    // Both magnitudes could change provided they stay in proportion, but
    //   here for convenience we make the direction a unit vector.
    player_dir << 0, 1;
    view_plane << 2.0/3, 0;
}

void DdaRaycastEngine::updateScreenSize(const uint16_t w) {
    screen_w = w;
    fov_rays.resize(screen_w);
}

// TBD: make layout member of engine?
void DdaRaycastEngine::castRay(const uint16_t screen_x,
                                 const Settings& settings) {
    // x coordinate in the camera plane represented by the current
    //   screen x coordinate, calculated so that the left edge of the
    //   camera plane is -1.0, center 0.0, and right edge is 1.0
    double camera_x { 2 * screen_x / double(screen_w) - 1 };

    FovRay& ray { fov_rays[screen_x] };

    // ray origin is player_pos
    // multiply camera_plane vector by scalar x, then add to direction vector
    //   to get ray direction
    ray.dir = player_dir + (view_plane * camera_x);

    // current map grid coordinates of ray
    uint16_t map_x ( player_pos(0) );
    uint16_t map_y ( player_pos(1) );

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
    double dist_per_unit_x { std::abs(1 / ray.dir(0)) };
    double dist_per_unit_y { std::abs(1 / ray.dir(1)) };

    // DDA algorithm will always jump exactly one map grid square each
    //   loop, either in the x or y. These vars record those increments,
    //   either -1 or +1
    int8_t map_step_x;
    int8_t map_step_y;

    // setup map grid step and initial ray distance to next grid unit values
    if (ray.dir(0) < 0) {
        map_step_x = -1;
        dist_next_unit_x = (player_pos(0) - map_x) * dist_per_unit_x;
    } else {
        map_step_x = 1;
        dist_next_unit_x = (map_x + 1.0 - player_pos(0)) * dist_per_unit_x;
    }
    if (ray.dir(1) < 0) {
        map_step_y = -1;
        dist_next_unit_y = (player_pos(1) - map_y) * dist_per_unit_y;
    } else {
        map_step_y = 1;
        dist_next_unit_y = (map_y + 1.0 - player_pos(1)) * dist_per_unit_y;
    }

    // perform DDA algo, or the incremental casting of the ray
    // moves to a new map unit square every loop, as directed by map_step values
    // TBD: does orientation need to be set every loop?
    while (!layout.tileIsWall(map_x, map_y)) {
        if (dist_next_unit_x < dist_next_unit_y) {
            dist_next_unit_x += dist_per_unit_x;
            map_x += map_step_x;
            ray.type_wall_hit = WallOrientation::EW;
        } else {
            dist_next_unit_y += dist_per_unit_y;
            map_y += map_step_y;
            ray.type_wall_hit = WallOrientation::NS;
        }
        // TBD: what about OneLoneCoder's max cast distance?
    }

    // ray.wall.map_x = map_x;
    // ray.wall.map_y = map_y;

    // Calculate distance to wall hit from camera plane, moving
    //   perpendicular to the camera plane. If the actual length of the
    //   ray cast from player to wall were used, the result would be
    //   a fisheye effect. For example, if the player were squarely
    //   facing a wall (wall parallel to camera plane,) in the resulting
    //   render the wall should appear of even height. If using the
    //   length of the rays as they hit the wall, the wall would instead
    //   appear to taper to the left and right ends of the display.
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

    // TBD: double check these
    if (settings.fisheye) {  // Euclidean ray distance from player_pos
        ray.wall_dist = (ray.type_wall_hit == WallOrientation::EW) ?
            dist_next_unit_x : dist_next_unit_y;
    } else {                // perpendicular distance from camera plane
        ray.wall_dist = (ray.type_wall_hit == WallOrientation::EW) ?
            dist_next_unit_x - dist_per_unit_x :
            dist_next_unit_y - dist_per_unit_y;
    }
}

void DdaRaycastEngine::castRays(const Settings& settings) {
    for (uint16_t screen_x { 0 }; screen_x < screen_w; ++screen_x) {
        castRay(screen_x, settings);
    }
}

// CCW
void DdaRaycastEngine::playerTurnLeft(const double rot_speed) {
    player_dir = rotateVector2d(player_dir, rot_speed);
    view_plane = rotateVector2d(view_plane, rot_speed);
}

// CW
void DdaRaycastEngine::playerTurnRight(const double rot_speed) {
    player_dir = rotateVector2d(player_dir, -rot_speed);
    view_plane = rotateVector2d(view_plane, -rot_speed);
}

// Vector2d is { x, y }
void DdaRaycastEngine::playerStrafeLeft(const double move_speed) {
    double start_ppx { player_pos(0) };
    if (!layout.tileIsWall(player_pos(0) - player_dir(1) * move_speed, player_pos(1)))
        player_pos(0) -= player_dir(1) * move_speed;
    if (!layout.tileIsWall(start_ppx, player_pos(1) + player_dir(0) * move_speed))
        player_pos(1) += player_dir(0) * move_speed;
}

void DdaRaycastEngine::playerStrafeRight(const double move_speed) {
    double start_ppx { player_pos(0) };
    if (!layout.tileIsWall(player_pos(0) + player_dir(1) * move_speed, player_pos(1)))
        player_pos(0) += player_dir(1) * move_speed;
    if (!layout.tileIsWall(start_ppx, player_pos(1) - player_dir(0) * move_speed))
        player_pos(1) -= player_dir(0) * move_speed;
}

// Vector2d is { x, y }
void DdaRaycastEngine::playerMoveFwd(const double move_speed) {
    double start_ppx { player_pos(0) };
    if (!layout.tileIsWall(player_pos(0) + player_dir(0) * move_speed, player_pos(1)))
        player_pos(0) += player_dir(0) * move_speed;
    if (!layout.tileIsWall(start_ppx, player_pos(1) + player_dir(1) * move_speed))
        player_pos(1) += player_dir(1) * move_speed;
}

void DdaRaycastEngine::playerMoveBack(const double move_speed) {
    double start_ppx { player_pos(0) };
    if (!layout.tileIsWall(player_pos(0) - player_dir(0) * move_speed, player_pos(1)))
        player_pos(0) -= player_dir(0) * move_speed;
    if (!layout.tileIsWall(start_ppx, player_pos(1) - player_dir(1) * move_speed))
        player_pos(1) -= player_dir(1) * move_speed;
}

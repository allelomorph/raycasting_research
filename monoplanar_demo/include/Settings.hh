#ifndef SETTINGS_HH
#define SETTINGS_HH


// TBD: reset defaults after development
struct Settings {
    bool     show_map            { true };

    // minimap height : screen height
    double   map_proportion      { 1.0/3 };
    // TBD: move map dims to DisplayMgr
    uint16_t map_h;
    uint16_t map_w;

    bool     show_fps            { false };
    bool     debug_mode          { true };

    // when true, use real ray distance to wall rather than perpendicular
    //   camera plane distance
    bool     fisheye             { false };

    // used to determine player movement speed, as pegged to frame rate
    double   base_movement_rate  { 5.0 };
    // expressed as percentage of base_movement_rate
    double   turn_rate           { 0.6 };  // 3.0
};


#endif  // SETTINGS_HH

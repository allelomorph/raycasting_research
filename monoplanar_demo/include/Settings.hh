#ifndef SETTINGS_HH
#define SETTINGS_HH


enum class TtyDisplayMode { Uninitialized, Ascii, ColorCode, TrueColor };

struct Settings {
    TtyDisplayMode  tty_display_mode    { TtyDisplayMode::Uninitialized };

    bool            show_map            { true };
    // minimap height : screen height
    double          map_proportion      { 1.0/3 };

    bool            show_fps            { true };

    bool            debug_mode          { false };
    // when true, use real ray distance to wall rather than perpendicular
    //   camera plane distance
    bool            euclidean           { false };
    // used to determine player movement speed, as pegged to frame rate
    double          base_movement_rate  { 5.0 };
    // expressed as percentage of base_movement_rate
    double          turn_rate           { 0.6 };  // 3.0
};


#endif  // SETTINGS_HH

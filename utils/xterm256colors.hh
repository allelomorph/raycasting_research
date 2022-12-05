#include <cstdint>
#include <iostream>
#include <iomanip>
#include <array>


struct Xterm256Color {
    uint8_t code;
    const char* name;
    uint32_t rgb;
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    friend std::ostream& operator<<(std::ostream& os, const Xterm256Color& xtc) {
        return os << std::setw(3) << std::dec << (int)xtc.code << " \"" <<
            xtc.name << "\"   \t0x" <<
            std::hex << xtc.rgb <<
            "     \t(" << std::dec << (int)xtc.red << ", " <<
            (int)xtc.green << ", " <<
            (int)xtc.blue << ")";
    }
};

// xterm 256 color code data from:
//   - https://www.ditig.com/256-colors-cheat-sheet

// emacs tools used to format data:
//     C-u M-x align-regexp ,\(\) 1 0 y
//     see: https://emacs.stackexchange.com/questions/2644/understanding-of-emacs-align-regexp
//
//     M-x replace-regexp (note capture groups are matched with \1, \2, rather than $1, $2)
//     see: https://stackoverflow.com/questions/2003774/emacs-how-to-replace-a-string-using-a-regular-expression

constexpr std::array<Xterm256Color, 256> xterm_256_colors { {
    // system colors
    { 0  , "Black (SYSTEM)"    , 0x000000 , 0  , 0  , 0 }  ,  // same as 16  Grey0
    { 1  , "Maroon (SYSTEM)"   , 0x800000 , 128, 0  , 0 }  ,
    { 2  , "Green (SYSTEM)"    , 0x008000 , 0  , 128, 0 }  ,
    { 3  , "Olive (SYSTEM)"    , 0x808000 , 128, 128, 0 }  ,
    { 4  , "Navy (SYSTEM)"     , 0x000080 , 0  , 0  , 128 },
    { 5  , "Purple (SYSTEM)"   , 0x800080 , 128, 0  , 128 },
    { 6  , "Teal (SYSTEM)"     , 0x008080 , 0  , 128, 128 },
    { 7  , "Silver (SYSTEM)"   , 0xc0c0c0 , 192, 192, 192 },
    { 8  , "Grey (SYSTEM)"     , 0x808080 , 128, 128, 128 },  // same as 244 Grey50
    { 9  , "Red (SYSTEM)"      , 0xff0000 , 255, 0  , 0 }  ,  // same as 196 Red1
    { 10 , "Lime (SYSTEM)"     , 0x00ff00 , 0  , 255, 0 }  ,  // same as 46  Green1
    { 11 , "Yellow (SYSTEM)"   , 0xffff00 , 255, 255, 0 }  ,  // same as 226 Yellow1
    { 12 , "Blue (SYSTEM)"     , 0x0000ff , 0  , 0  , 255 },  // same as 21  Blue1
    { 13 , "Fuchsia (SYSTEM)"  , 0xff00ff , 255, 0  , 255 },  // same as 201 Magenta1
    { 14 , "Aqua (SYSTEM)"     , 0x00ffff , 0  , 255, 255 },  // same as 51  Cyan1
    { 15 , "White (SYSTEM)"    , 0xffffff , 255, 255, 255 },  // same as 231 Grey100

    // color cube 1
    { 16 , "Grey0"             , 0x000000 , 0  , 0  , 0 }  ,  // same as 0 Black (SYSTEM)
    { 17 , "NavyBlue"          , 0x00005f , 0  , 0  , 95 } ,
    { 18 , "DarkBlue"          , 0x000087 , 0  , 0  , 135 },
    { 19 , "Blue3"             , 0x0000af , 0  , 0  , 175 },
    { 20 , "Blue3"             , 0x0000d7 , 0  , 0  , 215 },
    { 21 , "Blue1"             , 0x0000ff , 0  , 0  , 255 },  // same as 12 Blue (SYSTEM)
    { 22 , "DarkGreen"         , 0x005f00 , 0  , 95 , 0 }  ,
    { 23 , "DeepSkyBlue4"      , 0x005f5f , 0  , 95 , 95 } ,
    { 24 , "DeepSkyBlue4"      , 0x005f87 , 0  , 95 , 135 },
    { 25 , "DeepSkyBlue4"      , 0x005faf , 0  , 95 , 175 },
    { 26 , "DodgerBlue3"       , 0x005fd7 , 0  , 95 , 215 },
    { 27 , "DodgerBlue2"       , 0x005fff , 0  , 95 , 255 },
    { 28 , "Green4"            , 0x008700 , 0  , 135, 0 }  ,
    { 29 , "SpringGreen4"      , 0x00875f , 0  , 135, 95 } ,
    { 30 , "Turquoise4"        , 0x008787 , 0  , 135, 135 },
    { 31 , "DeepSkyBlue3"      , 0x0087af , 0  , 135, 175 },
    { 32 , "DeepSkyBlue3"      , 0x0087d7 , 0  , 135, 215 },
    { 33 , "DodgerBlue1"       , 0x0087ff , 0  , 135, 255 },
    { 34 , "Green3"            , 0x00af00 , 0  , 175, 0 }  ,
    { 35 , "SpringGreen3"      , 0x00af5f , 0  , 175, 95 } ,
    { 36 , "DarkCyan"          , 0x00af87 , 0  , 175, 135 },
    { 37 , "LightSeaGreen"     , 0x00afaf , 0  , 175, 175 },
    { 38 , "DeepSkyBlue2"      , 0x00afd7 , 0  , 175, 215 },
    { 39 , "DeepSkyBlue1"      , 0x00afff , 0  , 175, 255 },
    { 40 , "Green3"            , 0x00d700 , 0  , 215, 0 }  ,
    { 41 , "SpringGreen3"      , 0x00d75f , 0  , 215, 95 } ,
    { 42 , "SpringGreen2"      , 0x00d787 , 0  , 215, 135 },
    { 43 , "Cyan3"             , 0x00d7af , 0  , 215, 175 },
    { 44 , "DarkTurquoise"     , 0x00d7d7 , 0  , 215, 215 },
    { 45 , "Turquoise2"        , 0x00d7ff , 0  , 215, 255 },
    { 46 , "Green1"            , 0x00ff00 , 0  , 255, 0 }  ,  // same as 10  Lime (SYSTEM)
    { 47 , "SpringGreen2"      , 0x00ff5f , 0  , 255, 95 } ,
    { 48 , "SpringGreen1"      , 0x00ff87 , 0  , 255, 135 },
    { 49 , "MediumSpringGreen" , 0x00ffaf , 0  , 255, 175 },
    { 50 , "Cyan2"             , 0x00ffd7 , 0  , 255, 215 },
    { 51 , "Cyan1"             , 0x00ffff , 0  , 255, 255 },  // same as 14 Aqua (SYSTEM)

    // color cube 2
    { 52 , "DarkRed"           , 0x5f0000 , 95 , 0  , 0 }  ,
    { 53 , "DeepPink4"         , 0x5f005f , 95 , 0  , 95 } ,
    { 54 , "Purple4"           , 0x5f0087 , 95 , 0  , 135 },
    { 55 , "Purple4"           , 0x5f00af , 95 , 0  , 175 },
    { 56 , "Purple3"           , 0x5f00d7 , 95 , 0  , 215 },
    { 57 , "BlueViolet"        , 0x5f00ff , 95 , 0  , 255 },
    { 58 , "Orange4"           , 0x5f5f00 , 95 , 95 , 0 }  ,
    { 59 , "Grey37"            , 0x5f5f5f , 95 , 95 , 95 } ,
    { 60 , "MediumPurple4"     , 0x5f5f87 , 95 , 95 , 135 },
    { 61 , "SlateBlue3"        , 0x5f5faf , 95 , 95 , 175 },
    { 62 , "SlateBlue3"        , 0x5f5fd7 , 95 , 95 , 215 },
    { 63 , "RoyalBlue1"        , 0x5f5fff , 95 , 95 , 255 },
    { 64 , "Chartreuse4"       , 0x5f8700 , 95 , 135, 0 }  ,
    { 65 , "DarkSeaGreen4"     , 0x5f875f , 95 , 135, 95 } ,
    { 66 , "PaleTurquoise4"    , 0x5f8787 , 95 , 135, 135 },
    { 67 , "SteelBlue"         , 0x5f87af , 95 , 135, 175 },
    { 68 , "SteelBlue3"        , 0x5f87d7 , 95 , 135, 215 },
    { 69 , "CornflowerBlue"    , 0x5f87ff , 95 , 135, 255 },
    { 70 , "Chartreuse3"       , 0x5faf00 , 95 , 175, 0 }  ,
    { 71 , "DarkSeaGreen4"     , 0x5faf5f , 95 , 175, 95 } ,
    { 72 , "CadetBlue"         , 0x5faf87 , 95 , 175, 135 },
    { 73 , "CadetBlue"         , 0x5fafaf , 95 , 175, 175 },
    { 74 , "SkyBlue3"          , 0x5fafd7 , 95 , 175, 215 },
    { 75 , "SteelBlue1"        , 0x5fafff , 95 , 175, 255 },
    { 76 , "Chartreuse3"       , 0x5fd700 , 95 , 215, 0 }  ,
    { 77 , "PaleGreen3"        , 0x5fd75f , 95 , 215, 95 } ,
    { 78 , "SeaGreen3"         , 0x5fd787 , 95 , 215, 135 },
    { 79 , "Aquamarine3"       , 0x5fd7af , 95 , 215, 175 },
    { 80 , "MediumTurquoise "  , 0x5fd7d7 , 95 , 215, 215 },
    { 81 , "SteelBlue1"        , 0x5fd7ff , 95 , 215, 255 },
    { 82 , "Chartreuse2"       , 0x5fff00 , 95 , 255, 0 }  ,
    { 83 , "SeaGreen2"         , 0x5fff5f , 95 , 255, 95 } ,
    { 84 , "SeaGreen1"         , 0x5fff87 , 95 , 255, 135 },
    { 85 , "SeaGreen1"         , 0x5fffaf , 95 , 255, 175 },
    { 86 , "Aquamarine1"       , 0x5fffd7 , 95 , 255, 215 },
    { 87 , "DarkSlateGray2"    , 0x5fffff , 95 , 255, 255 },

    // color cube 3
    { 88 , "DarkRed"           , 0x870000 , 135, 0  , 0 }  ,
    { 89 , "DeepPink4"         , 0x87005f , 135, 0  , 95 } ,
    { 90 , "DarkMagenta"       , 0x870087 , 135, 0  , 135 },
    { 91 , "DarkMagenta"       , 0x8700af , 135, 0  , 175 },
    { 92 , "DarkViolet"        , 0x8700d7 , 135, 0  , 215 },
    { 93 , "Purple"            , 0x8700ff , 135, 0  , 255 },
    { 94 , "Orange4"           , 0x875f00 , 135, 95 , 0 }  ,
    { 95 , "LightPink4"        , 0x875f5f , 135, 95 , 95 } ,
    { 96 , "Plum4"             , 0x875f87 , 135, 95 , 135 },
    { 97 , "MediumPurple3"     , 0x875faf , 135, 95 , 175 },
    { 98 , "MediumPurple3"     , 0x875fd7 , 135, 95 , 215 },
    { 99 , "SlateBlue1"        , 0x875fff , 135, 95 , 255 },
    { 100, "Yellow4"           , 0x878700 , 135, 135, 0 }  ,
    { 101, "Wheat4"            , 0x87875f , 135, 135, 95 } ,
    { 102, "Grey53"            , 0x878787 , 135, 135, 135 },
    { 103, "LightSlateGrey"    , 0x8787af , 135, 135, 175 },
    { 104, "MediumPurple"      , 0x8787d7 , 135, 135, 215 },
    { 105, "LightSlateBlue"    , 0x8787ff , 135, 135, 255 },
    { 106, "Yellow4"           , 0x87af00 , 135, 175, 0 }  ,
    { 107, "DarkOliveGreen3"   , 0x87af5f , 135, 175, 95 } ,
    { 108, "DarkSeaGreen"      , 0x87af87 , 135, 175, 135 },
    { 109, "LightSkyBlue3"     , 0x87afaf , 135, 175, 175 },
    { 110, "LightSkyBlue3"     , 0x87afd7 , 135, 175, 215 },
    { 111, "SkyBlue2"          , 0x87afff , 135, 175, 255 },
    { 112, "Chartreuse2"       , 0x87d700 , 135, 215, 0 }  ,
    { 113, "DarkOliveGreen3"   , 0x87d75f , 135, 215, 95 } ,
    { 114, "PaleGreen3"        , 0x87d787 , 135, 215, 135 },
    { 115, "DarkSeaGreen3"     , 0x87d7af , 135, 215, 175 },
    { 116, "DarkSlateGray3"    , 0x87d7d7 , 135, 215, 215 },
    { 117, "SkyBlue1"          , 0x87d7ff , 135, 215, 255 },
    { 118, "Chartreuse1"       , 0x87ff00 , 135, 255, 0 }  ,
    { 119, "LightGreen"        , 0x87ff5f , 135, 255, 95 } ,
    { 120, "LightGreen"        , 0x87ff87 , 135, 255, 135 },
    { 121, "PaleGreen1"        , 0x87ffaf , 135, 255, 175 },
    { 122, "Aquamarine1"       , 0x87ffd7 , 135, 255, 215 },
    { 123, "DarkSlateGray1"    , 0x87ffff , 135, 255, 255 },

    // color cube 4
    { 124, "Red3"              , 0xaf0000 , 175, 0  , 0 }  ,
    { 125, "DeepPink4"         , 0xaf005f , 175, 0  , 95 } ,
    { 126, "MediumVioletRed"   , 0xaf0087 , 175, 0  , 135 },
    { 127, "Magenta3"          , 0xaf00af , 175, 0  , 175 },
    { 128, "DarkViolet"        , 0xaf00d7 , 175, 0  , 215 },
    { 129, "Purple"            , 0xaf00ff , 175, 0  , 255 },
    { 130, "DarkOrange3"       , 0xaf5f00 , 175, 95 , 0 }  ,
    { 131, "IndianRed"         , 0xaf5f5f , 175, 95 , 95 } ,
    { 132, "HotPink3"          , 0xaf5f87 , 175, 95 , 135 },
    { 133, "MediumOrchid3"     , 0xaf5faf , 175, 95 , 175 },
    { 134, "MediumOrchid"      , 0xaf5fd7 , 175, 95 , 215 },
    { 135, "MediumPurple2"     , 0xaf5fff , 175, 95 , 255 },
    { 136, "DarkGoldenrod"     , 0xaf8700 , 175, 135, 0 }  ,
    { 137, "LightSalmon3"      , 0xaf875f , 175, 135, 95 } ,
    { 138, "RosyBrown"         , 0xaf8787 , 175, 135, 135 },
    { 139, "Grey63"            , 0xaf87af , 175, 135, 175 },
    { 140, "MediumPurple2"     , 0xaf87d7 , 175, 135, 215 },
    { 141, "MediumPurple1"     , 0xaf87ff , 175, 135, 255 },
    { 142, "Gold3"             , 0xafaf00 , 175, 175, 0 }  ,
    { 143, "DarkKhaki"         , 0xafaf5f , 175, 175, 95 } ,
    { 144, "NavajoWhite3"      , 0xafaf87 , 175, 175, 135 },
    { 145, "Grey69"            , 0xafafaf , 175, 175, 175 },
    { 146, "LightSteelBlue3"   , 0xafafd7 , 175, 175, 215 },
    { 147, "LightSteelBlue"    , 0xafafff , 175, 175, 255 },
    { 148, "Yellow3"           , 0xafd700 , 175, 215, 0 }  ,
    { 149, "DarkOliveGreen3"   , 0xafd75f , 175, 215, 95 } ,
    { 150, "DarkSeaGreen3"     , 0xafd787 , 175, 215, 135 },
    { 151, "DarkSeaGreen2"     , 0xafd7af , 175, 215, 175 },
    { 152, "LightCyan3"        , 0xafd7d7 , 175, 215, 215 },
    { 153, "LightSkyBlue1"     , 0xafd7ff , 175, 215, 255 },
    { 154, "GreenYellow"       , 0xafff00 , 175, 255, 0 }  ,
    { 155, "DarkOliveGreen2"   , 0xafff5f , 175, 255, 95 } ,
    { 156, "PaleGreen1"        , 0xafff87 , 175, 255, 135 },
    { 157, "DarkSeaGreen2"     , 0xafffaf , 175, 255, 175 },
    { 158, "DarkSeaGreen1"     , 0xafffd7 , 175, 255, 215 },
    { 159, "PaleTurquoise1"    , 0xafffff , 175, 255, 255 },

    // color cube 5
    { 160, "Red3"              , 0xd70000 , 215, 0  , 0 }  ,
    { 161, "DeepPink3"         , 0xd7005f , 215, 0  , 95 } ,
    { 162, "DeepPink3"         , 0xd70087 , 215, 0  , 135 },
    { 163, "Magenta3"          , 0xd700af , 215, 0  , 175 },
    { 164, "Magenta3"          , 0xd700d7 , 215, 0  , 215 },
    { 165, "Magenta2"          , 0xd700ff , 215, 0  , 255 },
    { 166, "DarkOrange3"       , 0xd75f00 , 215, 95 , 0 }  ,
    { 167, "IndianRed"         , 0xd75f5f , 215, 95 , 95 } ,
    { 168, "HotPink3"          , 0xd75f87 , 215, 95 , 135 },
    { 169, "HotPink2"          , 0xd75faf , 215, 95 , 175 },
    { 170, "Orchid"            , 0xd75fd7 , 215, 95 , 215 },
    { 171, "MediumOrchid1"     , 0xd75fff , 215, 95 , 255 },
    { 172, "Orange3"           , 0xd78700 , 215, 135, 0 }  ,
    { 173, "LightSalmon3"      , 0xd7875f , 215, 135, 95 } ,
    { 174, "LightPink3"        , 0xd78787 , 215, 135, 135 },
    { 175, "Pink3"             , 0xd787af , 215, 135, 175 },
    { 176, "Plum3"             , 0xd787d7 , 215, 135, 215 },
    { 177, "Violet"            , 0xd787ff , 215, 135, 255 },
    { 178, "Gold3"             , 0xd7af00 , 215, 175, 0 }  ,
    { 179, "LightGoldenrod3"   , 0xd7af5f , 215, 175, 95 } ,
    { 180, "Tan"               , 0xd7af87 , 215, 175, 135 },
    { 181, "MistyRose3"        , 0xd7afaf , 215, 175, 175 },
    { 182, "Thistle3"          , 0xd7afd7 , 215, 175, 215 },
    { 183, "Plum2"             , 0xd7afff , 215, 175, 255 },
    { 184, "Yellow3"           , 0xd7d700 , 215, 215, 0 }  ,
    { 185, "Khaki3"            , 0xd7d75f , 215, 215, 95 } ,
    { 186, "LightGoldenrod2"   , 0xd7d787 , 215, 215, 135 },
    { 187, "LightYellow3"      , 0xd7d7af , 215, 215, 175 },
    { 188, "Grey84"            , 0xd7d7d7 , 215, 215, 215 },
    { 189, "LightSteelBlue1"   , 0xd7d7ff , 215, 215, 255 },
    { 190, "Yellow2"           , 0xd7ff00 , 215, 255, 0 }  ,
    { 191, "DarkOliveGreen1"   , 0xd7ff5f , 215, 255, 95 } ,
    { 192, "DarkOliveGreen1"   , 0xd7ff87 , 215, 255, 135 },
    { 193, "DarkSeaGreen1"     , 0xd7ffaf , 215, 255, 175 },
    { 194, "Honeydew2"         , 0xd7ffd7 , 215, 255, 215 },
    { 195, "LightCyan1"        , 0xd7ffff , 215, 255, 255 },

    // color cube 6
    { 196, "Red1"              , 0xff0000 , 255, 0  , 0 }  ,  // same as 9 Red (SYSTEM)
    { 197, "DeepPink2"         , 0xff005f , 255, 0  , 95 } ,
    { 198, "DeepPink1"         , 0xff0087 , 255, 0  , 135 },
    { 199, "DeepPink1"         , 0xff00af , 255, 0  , 175 },
    { 200, "Magenta2"          , 0xff00d7 , 255, 0  , 215 },
    { 201, "Magenta1"          , 0xff00ff , 255, 0  , 255 },  // same as 13 Fuschia (SYSTEM)
    { 202, "OrangeRed1"        , 0xff5f00 , 255, 95 , 0 }  ,
    { 203, "IndianRed1"        , 0xff5f5f , 255, 95 , 95 } ,
    { 204, "IndianRed1"        , 0xff5f87 , 255, 95 , 135 },
    { 205, "HotPink"           , 0xff5faf , 255, 95 , 175 },
    { 206, "HotPink"           , 0xff5fd7 , 255, 95 , 215 },
    { 207, "MediumOrchid1"     , 0xff5fff , 255, 95 , 255 },
    { 208, "DarkOrange"        , 0xff8700 , 255, 135, 0 }  ,
    { 209, "Salmon1"           , 0xff875f , 255, 135, 95 } ,
    { 210, "LightCoral"        , 0xff8787 , 255, 135, 135 },
    { 211, "PaleVioletRed1"    , 0xff87af , 255, 135, 175 },
    { 212, "Orchid2"           , 0xff87d7 , 255, 135, 215 },
    { 213, "Orchid1"           , 0xff87ff , 255, 135, 255 },
    { 214, "Orange1"           , 0xffaf00 , 255, 175, 0 }  ,
    { 215, "SandyBrown"        , 0xffaf5f , 255, 175, 95 } ,
    { 216, "LightSalmon1"      , 0xffaf87 , 255, 175, 135 },
    { 217, "LightPink1"        , 0xffafaf , 255, 175, 175 },
    { 218, "Pink1"             , 0xffafd7 , 255, 175, 215 },
    { 219, "Plum1"             , 0xffafff , 255, 175, 255 },
    { 220, "Gold1"             , 0xffd700 , 255, 215, 0 }  ,
    { 221, "LightGoldenrod2"   , 0xffd75f , 255, 215, 95 } ,
    { 222, "LightGoldenrod2"   , 0xffd787 , 255, 215, 135 },
    { 223, "NavajoWhite1"      , 0xffd7af , 255, 215, 175 },
    { 224, "MistyRose1"        , 0xffd7d7 , 255, 215, 215 },
    { 225, "Thistle1"          , 0xffd7ff , 255, 215, 255 },
    { 226, "Yellow1"           , 0xffff00 , 255, 255, 0 }  ,  // same as 11 Yellow (SYSTEM)
    { 227, "LightGoldenrod1"   , 0xffff5f , 255, 255, 95 } ,
    { 228, "Khaki1"            , 0xffff87 , 255, 255, 135 },
    { 229, "Wheat1"            , 0xffffaf , 255, 255, 175 },
    { 230, "Cornsilk1"         , 0xffffd7 , 255, 255, 215 },
    { 231, "Grey100"           , 0xffffff , 255, 255, 255 },  // same as 15 White (SYSTEM)

    // remaining greys not appearing in color cubes
    { 232, "Grey3"             , 0x080808 , 8  , 8  , 8 }  ,
    { 233, "Grey7"             , 0x121212 , 18 , 18 , 18 } ,
    { 234, "Grey11"            , 0x1c1c1c , 28 , 28 , 28 } ,
    { 235, "Grey15"            , 0x262626 , 38 , 38 , 38 } ,
    { 236, "Grey19"            , 0x303030 , 48 , 48 , 48 } ,
    { 237, "Grey23"            , 0x3a3a3a , 58 , 58 , 58 } ,
    { 238, "Grey27"            , 0x444444 , 68 , 68 , 68 } ,
    { 239, "Grey30"            , 0x4e4e4e , 78 , 78 , 78 } ,
    { 240, "Grey35"            , 0x585858 , 88 , 88 , 88 } ,
    { 241, "Grey39"            , 0x626262 , 98 , 98 , 98 } ,
    { 242, "Grey42"            , 0x6c6c6c , 108, 108, 108 },
    { 243, "Grey46"            , 0x767676 , 118, 118, 118 },
    { 244, "Grey50"            , 0x808080 , 128, 128, 128 },  // same as 8 Grey (SYSTEM)
    { 245, "Grey54"            , 0x8a8a8a , 138, 138, 138 },
    { 246, "Grey58"            , 0x949494 , 148, 148, 148 },
    { 247, "Grey62"            , 0x9e9e9e , 158, 158, 158 },
    { 248, "Grey66"            , 0xa8a8a8 , 168, 168, 168 },
    { 249, "Grey70"            , 0xb2b2b2 , 178, 178, 178 },
    { 250, "Grey74"            , 0xbcbcbc , 188, 188, 188 },
    { 251, "Grey78"            , 0xc6c6c6 , 198, 198, 198 },
    { 252, "Grey82"            , 0xd0d0d0 , 208, 208, 208 },
    { 253, "Grey85"            , 0xdadada , 218, 218, 218 },
    { 254, "Grey89"            , 0xe4e4e4 , 228, 228, 228 },
    { 255, "Grey93"            , 0xeeeeee , 238, 238, 238 }
    } };

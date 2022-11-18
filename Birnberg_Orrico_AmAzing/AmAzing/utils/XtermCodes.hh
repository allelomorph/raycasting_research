/*
 * @file XtermCodes namespace contains functions to generate terminal escape and
 *   control sequence initiator (CSI) codes for adjusting terminal settings.
 */

#ifndef XTERMCODES_HH
#define XTERMCODES_HH


#include <cstdint>

#include <iostream>


namespace XtermCodes {

// enumerates the 16 default xterm system colors
enum class SysColor {
    Black, Red, Green, Yellow, Blue, Magenta, Cyan, White,
    LightBlack, LightRed, LightGreen, LightYellow, LightBlue, LightMagenta, LightCyan, LightWhite
};

// XtermCodes classes used to insert into stream as if called like an iomanip
// TBD: can C++20's format help avoid need to cast to ints before streaming?

class charFgColor {
private:
    bool true_color;
    union {
        uint8_t color;  // 256-color mode
        uint8_t red;    // 3-byte truecolor
    };
    uint8_t green;      // 3-byte truecolor
    uint8_t blue;       // 3-byte truecolor
public:
    charFgColor(SysColor c) :
        true_color(false), color(static_cast<uint8_t>(c)) {}
    charFgColor(uint8_t c) :
        true_color(false), color(c) {}
    charFgColor(uint8_t r, uint8_t g, uint8_t b) :
        true_color(true), red(r), green(g), blue(b) {}

    friend std::ostream& operator<<(std::ostream& os, const charFgColor& cfgc);
};

class charBgColor {
private:
    bool true_color;
    union {
        uint8_t color;  // 256-color mode
        uint8_t red;    // 3-byte truecolor
    };
    uint8_t green;      // 3-byte truecolor
    uint8_t blue;       // 3-byte truecolor
public:
    charBgColor(SysColor c) :
        true_color(false), color(static_cast<uint8_t>(c)) {}
    charBgColor(uint8_t c) :
        true_color(false), color(c) {}
    charBgColor(uint8_t r, uint8_t g, uint8_t b) :
        true_color(true), red(r), green(g), blue(b) {}

    friend std::ostream& operator<<(std::ostream& os, const charBgColor& cbgc);
};

class charDefaults {
public:
    friend std::ostream& operator<<(std::ostream& os, const charDefaults& /*cdflts*/);
};

}  // namespace XtermCodes


#endif  //  XTERMCODES_HH

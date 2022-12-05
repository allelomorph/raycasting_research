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
// system color names from:
//   - https://www.ditig.com/256-colors-cheat-sheet
// note: in testing, 8-color terminals apparently use the first 8 for the second 8
enum class SysColor {
    Black, Maroon, Green, Olive, Navy, Purple, Teal, Silver,
    Grey, Red, Lime, Yellow, Blue, Fuschia, Aqua, White
};

uint8_t colorCodeFromRGB(uint8_t red, uint8_t green, uint8_t blue);

// XtermCodes classes used to insert into stream as if called like an iomanip
// TBD: can C++20's format help avoid need to cast to ints before streaming?

class CharFgColor {
private:
    bool true_color;
    union {
        uint8_t color;  // 256-color mode
        uint8_t red;    // 3-byte truecolor
    };
    uint8_t green;      // 3-byte truecolor
    uint8_t blue;       // 3-byte truecolor
public:
    CharFgColor(SysColor c) :
        true_color(false), color(static_cast<uint8_t>(c)) {}
    CharFgColor(uint8_t c) :
        true_color(false), color(c) {}
    CharFgColor(uint8_t r, uint8_t g, uint8_t b) :
        true_color(true), red(r), green(g), blue(b) {}

    friend std::ostream& operator<<(std::ostream& os, const CharFgColor& cfgc);
};

class CharBgColor {
private:
    bool true_color;
    union {
        uint8_t color;  // 256-color mode
        uint8_t red;    // 3-byte truecolor
    };
    uint8_t green;      // 3-byte truecolor
    uint8_t blue;       // 3-byte truecolor
public:
    CharBgColor(SysColor c) :
        true_color(false), color(static_cast<uint8_t>(c)) {}
    CharBgColor(uint8_t c) :
        true_color(false), color(c) {}
    CharBgColor(uint8_t r, uint8_t g, uint8_t b) :
        true_color(true), red(r), green(g), blue(b) {}

    friend std::ostream& operator<<(std::ostream& os, const CharBgColor& cbgc);
};

class CharDefaults {
public:
    friend std::ostream& operator<<(std::ostream& os, const CharDefaults& /*cdflts*/);
};

class CursorUp {
private:
    uint16_t rows;
public:
    CursorUp(uint16_t r) : rows(r) {}

    friend std::ostream& operator<<(std::ostream& os, const CursorUp& cu);
};

class CursorHome {
private:
    uint16_t row;
    uint16_t column;
public:
    CursorHome() : row(1), column(1) {}
    CursorHome(uint16_t r, uint16_t c) : row(r), column(c) {}

    friend std::ostream& operator<<(std::ostream& os, const CursorHome& ch);
};

class EraseLinesBelow {
public:
    friend std::ostream& operator<<(std::ostream& os, const EraseLinesBelow& /*elb*/);
};

class HideCursor {
public:
    friend std::ostream& operator<<(std::ostream& os, const HideCursor& /*hc*/);
};

class ShowCursor {
public:
    friend std::ostream& operator<<(std::ostream& os, const ShowCursor& /*sc*/);
};

}  // namespace XtermCodes


#endif  //  XTERMCODES_HH

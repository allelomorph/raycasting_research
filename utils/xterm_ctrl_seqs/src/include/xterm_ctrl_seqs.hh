/*
 * @file Xterm::CtrlSeqs namespace contains functions to generate terminal escape and
 *   control sequence initiator (CSI) codes for adjusting terminal settings.
 */

#ifndef XTERM_HH
#define XTERM_HH

#include <cstdint>

#include <iostream>


namespace Xterm {

namespace Color {

struct Data {
    union {
        uint8_t code;         // 256-color mode
        uint8_t r { 0 };      // 3-byte truecolor
    };
    uint8_t g { 0 };          // 3-byte truecolor
    uint8_t b { 0 };          // 3-byte truecolor

    Data() {}
    Data(const uint8_t _code) : code(_code) {}
    Data(const uint8_t _r, const uint8_t _g,
         const uint8_t _b) : r(_r), g(_g), b(_b) {}
};

namespace Codes {

// enumerates the 16 default xterm system colors
// system color names from:
//   - https://www.ditig.com/256-colors-cheat-sheet
// note: in testing, 8-color terminals apparently use the first 8 for the second 8
enum class System {
    Black, Maroon, Green, Olive, Navy, Purple, Teal, Silver,
    Grey, Red, Lime, Yellow, Blue, Fuschia, Aqua, White
};

// finds closest color code in 1 byte color mode palette from RGB
uint8_t fromRGB(const uint8_t r, const uint8_t g, const uint8_t b);

}  // namespace Codes

}  // namespace Color

namespace CtrlSeqs {

// Xterm::CtrlSeqs classes used to insert into stream as if called like an iomanip

class CharFgColor : private Color::Data {
private:
    bool true_color;
public:
    CharFgColor(const Color::Codes::System code) :
        Color::Data(static_cast<uint8_t>(code)), true_color(false) {}
    CharFgColor(const uint8_t code) :
        Color::Data(code), true_color(false) {}
    CharFgColor(const uint8_t r, const uint8_t g, const uint8_t b) :
        Color::Data(r, g, b), true_color(true) {}

    friend std::ostream& operator<<(std::ostream& os, const CharFgColor& cfgc);
};

class CharBgColor : private Color::Data {
private:
    bool true_color;
public:
    CharBgColor(const Color::Codes::System code) :
        Color::Data(static_cast<uint8_t>(code)), true_color(false) {}
    CharBgColor(const uint8_t code) :
        Color::Data(code), true_color(false) {}
    CharBgColor(const uint8_t r, const uint8_t g, const uint8_t b) :
        Color::Data(r, g, b), true_color(true) {}

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
    CursorHome(const uint16_t r, const uint16_t c) : row(r), column(c) {}

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

}  // namespace CtrlSeqs

}  // namespace Xterm


#endif  //  XTERM_HH

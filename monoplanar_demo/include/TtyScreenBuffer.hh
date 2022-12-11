#ifndef TTYSCREENBUFFER_HH
#define TTYSCREENBUFFER_HH

#include "Xterm.hh"     //  Color::Data

#include <cstdint>

#include <vector>


using PxColor = Xterm::Color::Data;

struct TtyPixel : public PxColor {
    char c { ' ' };

    TtyPixel() {}
    TtyPixel(const uint8_t _c, const uint8_t _code) :
        PxColor(_code), c(_c) {}
    TtyPixel(const uint8_t _c, const uint8_t _r, const uint8_t _g, const uint8_t _b) :
        PxColor(_r, _g, _b), c(_c) {}
};

class TtyScreenBuffer {
private:
    std::vector<TtyPixel> pixels;

public:
    uint16_t w;  // columns
    uint16_t h;  // rows

    // adjusting to terminal resize
    void resize(const uint16_t _w, const uint16_t _h);
    void resize(const uint16_t _w, const uint16_t _h, const TtyPixel px);

    // get/set pixels
    TtyPixel& pixel(const uint16_t col_i, const uint16_t row_i);
    char&     pixelChar(const uint16_t col_i, const uint16_t row_i);
    uint8_t&  pixelColorCode(const uint16_t col_i, const uint16_t row_i);

    // iterators to rows (when printing buffer to terminal)
    TtyPixel* rowBegin(const uint16_t row_i);
    TtyPixel* rowEnd(const uint16_t row_i);
};


#endif  // TTYSCREENBUFFER_HH

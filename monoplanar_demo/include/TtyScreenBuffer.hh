#ifndef TTYSCREENBUFFER_HH
#define TTYSCREENBUFFER_HH

#include "Xterm.hh"     //  Color::Data

#include <cstdint>

#include <vector>


using BgColorData = Xterm::Color::Data;

struct TtyPixel : public BgColorData {
    char c { ' ' };

    TtyPixel() {}
    TtyPixel(const uint8_t _c, const uint8_t _code) :
        BgColorData(_code), c(_c) {}
    TtyPixel(const uint8_t _c, const uint8_t r, const uint8_t g, const uint8_t b) :
        BgColorData(r, g, b), c(_c) {}
};

class TtyScreenBuffer {
private:
    std::vector<TtyPixel> pixels;
    // using tty_pixel_it_type = decltype(pixels.begin());

public:
    uint16_t w;  // columns
    uint16_t h;  // rows

    using tty_pixel_it_type = decltype(pixels.begin());

    // adjusting to terminal resize
    void resize(const uint16_t _w, const uint16_t _h);
    void resize(const uint16_t _w, const uint16_t _h, const TtyPixel px);

    // get/set pixels
    TtyPixel& pixel(const uint16_t col_i, const uint16_t row_i);
    char&     pixelChar(const uint16_t col_i, const uint16_t row_i);
    uint8_t&  pixelColorCode(const uint16_t col_i, const uint16_t row_i);

    // iterators to rows (when printing buffer to terminal)
    tty_pixel_it_type rowBegin(const uint16_t row_i);
    tty_pixel_it_type rowEnd(const uint16_t row_i);

    // horizontally fills in pixel chars, used to render minimap and HUD
    void pixelCharReplace(const uint16_t col_i, const uint16_t row_i,
                          const char* s, const uint16_t sz);
};


#endif  // TTYSCREENBUFFER_HH

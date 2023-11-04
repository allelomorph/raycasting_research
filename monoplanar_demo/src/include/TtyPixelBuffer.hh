#ifndef TTYPIXELBUFFER_HH
#define TTYPIXELBUFFER_HH

#include "Xterm.hh"     //  Color::Data

#include <cstdint>

#include <vector>


// tty pixels only need to record char background color, which is used as
//   pixel color with default ' '
using BgColorData = Xterm::Color::Data;

struct TtyPixel : public BgColorData {
    char c { ' ' };

    TtyPixel() {}
    TtyPixel(const uint8_t _c, const uint8_t code) :
        BgColorData(code), c(_c) {}
    TtyPixel(const uint8_t _c, const uint8_t r, const uint8_t g, const uint8_t b) :
        BgColorData(r, g, b), c(_c) {}
};

// TBD: after further testing, using a raw vector data() pointer outperforms both
//   pixel(x,y) and (by a wide margin) iterators - rewrite hotspot documentation
//   to reflect this
// TBD: Since in testing a 1D vector outperformed a 2D vector here, maybe create a
//   base template class GridBuffer<T> to also take the place of Layout.map,
//   as in theory this means tileData() and tileIsWall() use in raycasting and
//   keyboard input processing could also be a hotspot
// Pixel data getting and setting is an obvious performance bottleneck, so
//   several different options were tested for pixels and pixel(). The results
//   showed the best performance with:
//     - 1D vector for pixels, pixel() calculates index every time
//     - pixel type is plain data struct TtyPixel, values accessed through normal `.`
//     - pixel() is not inline (!)
//     - no safety assert() to check params in pixel()
//     - when traversing grid in the x in TtyDisplayMgr::drawScreen(), much
//         faster to use pixel() than iterators constructed from pixels.begin()

class TtyPixelBuffer {
private:
    std::vector<TtyPixel> pixels;

public:
    uint16_t w;  // columns
    uint16_t h;  // rows

    TtyPixelBuffer() {}
    TtyPixelBuffer(const uint16_t _w, const uint16_t _h) :
        pixels(std::vector<TtyPixel>(_w * _h)), w(_w), h(_h) {}

    // get/set pixels
    TtyPixel* pixel(const uint16_t col_i, const uint16_t row_i);

    // horizontally fills in pixel chars, used to render minimap and HUD
    void pixelCharReplace(const uint16_t col_i, const uint16_t row_i,
                          const char* s, const uint16_t sz);
};


#endif  // TTYPIXELBUFFER_HH

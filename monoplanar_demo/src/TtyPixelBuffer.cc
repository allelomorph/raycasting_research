#include "TtyPixelBuffer.hh"

#include <cassert>
#include <cstdint>


TtyPixel* TtyPixelBuffer::pixel(const uint16_t col_i, const uint16_t row_i) {
    // assert(col_i < w && row_i < h);
    return pixels.data() + ((row_i * w) + col_i);
}

void TtyPixelBuffer::pixelCharReplace(const uint16_t col_i, const uint16_t row_i,
                                       const char* s, const uint16_t sz) {
    TtyPixel* _pixel { pixel(col_i, row_i) };
    for (uint16_t i { 0 }; i < sz; ++i, ++_pixel) {
        _pixel->c = s[i];
    }
}

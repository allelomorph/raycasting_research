#include "TtyPixelBuffer.hh"

#include <cassert>
#include <cstdint>


TtyPixel* TtyPixelBuffer::pixel(const uint16_t col_i, const uint16_t row_i) {
    // assert(col_i < w && row_i < h);
    return pixels.data() + ((row_i * w) + col_i);
}

void TtyPixelBuffer::pixelCharReplace(const uint16_t col_i, const uint16_t row_i,
                                      const char* s, const uint16_t sz) {
    TtyPixel* px { pixel(col_i, row_i) };
    // new_px color never set, always default black background
    TtyPixel new_px;
    for (uint16_t i { 0 }; i < sz; ++i, ++px) {
        new_px.c = s[i];
        *px = new_px;
    }
}

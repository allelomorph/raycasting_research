#include "TtyScreenBuffer.hh"

#include <cassert>
#include <cstdint>


void TtyScreenBuffer::resize(const uint16_t _w, const uint16_t _h) {
    w = _w;
    h = _h;
    pixels.resize(w * h);
}

TtyPixel& TtyScreenBuffer::pixel(const uint16_t col_i, const uint16_t row_i) {
    // assert(col_i < w && row_i < h);
    return pixels[(row_i * w) + col_i];
}

void TtyScreenBuffer::pixelCharReplace(const uint16_t col_i, const uint16_t row_i,
                                       const char* s, const uint16_t sz) {
    assert(col_i < w && row_i < h);
    uint16_t offset ( (row_i * w) + col_i );
    assert(offset + sz < pixels.size());
    for (uint16_t i { 0 }; i < sz; ++i) {
        pixels[offset + i].c = s[i];
    }
}

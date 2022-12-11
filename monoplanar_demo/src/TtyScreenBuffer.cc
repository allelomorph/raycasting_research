#include "TtyScreenBuffer.hh"

#include <cassert>


void TtyScreenBuffer::resize(const uint16_t _w, const uint16_t _h) {
    w = _w;
    h = _h;
    pixels.resize(w * h);
}

void TtyScreenBuffer::resize(const uint16_t _w, const uint16_t _h,
                             const TtyPixel px) {
    w = _w;
    h = _h;
    data.resize(w * h, px);
}

TtyPixel& TtyScreenBuffer::pixel(const uint16_t col_i, const uint16_t row_i) {
    assert(col_i < w && row_i < h);
    return pixels[(row_i * w) + col_i];
}

char& TtyScreenBuffer::pixelChar(const uint16_t col_i, const uint16_t row_i) {
    assert(col_i < w && row_i < h);
    return data[(row_i * w) + col_i].c;
}

PxColor& TtyScreenBuffer::pixelColorCode(const uint16_t col_i,
                                         const uint16_t row_i) {
    assert(col_i < w && row_i < h);
    return data[(row_i * w) + col_i].code;
}

TtyPixel* TtyScreenBuffer::rowBegin(const uint16_t row_i) {
   assert(row_i < h);
   return pixels.begin() + (row_i * w);
}

TtyPixel* TtyScreenBuffer::rowEnd(const uint16_t row_i) {
   assert(row_i < h);
   return pixels.begin() + (row_i * (w + 1));
}

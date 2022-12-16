#include "TtyScreenBuffer.hh"

#include <cassert>
#include <cstdint>


void TtyScreenBuffer::resize(const uint16_t _w, const uint16_t _h) {
    w = _w;
    h = _h;
    pixels.resize(w * h);
}

void TtyScreenBuffer::resize(const uint16_t _w, const uint16_t _h,
                             const TtyPixel px) {
    w = _w;
    h = _h;
    pixels.resize(w * h, px);
}

TtyPixel& TtyScreenBuffer::pixel(const uint16_t col_i, const uint16_t row_i) {
    assert(col_i < w && row_i < h);
    return pixels[(row_i * w) + col_i];
}

char& TtyScreenBuffer::pixelChar(const uint16_t col_i, const uint16_t row_i) {
    assert(col_i < w && row_i < h);
    return pixels[(row_i * w) + col_i].c;
}

uint8_t& TtyScreenBuffer::pixelColorCode(const uint16_t col_i,
                                          const uint16_t row_i) {
    assert(col_i < w && row_i < h);
    return pixels[(row_i * w) + col_i].code;
}

using tty_pixel_it_type = typename TtyScreenBuffer::tty_pixel_it_type;

tty_pixel_it_type TtyScreenBuffer::rowBegin(const uint16_t row_i) {
    assert(row_i < h);
    return pixels.begin() + (row_i * w);
}

tty_pixel_it_type TtyScreenBuffer::rowEnd(const uint16_t row_i) {
    assert(row_i < h);
    return pixels.begin() + (row_i * (w + 1));
}

void TtyScreenBuffer::pixelCharReplace(const uint16_t col_i, const uint16_t row_i,
                                       const char* s, const uint16_t sz) {
    assert(col_i < w && row_i < h);
    uint16_t offset ( (row_i * w) + col_i );
    assert(offset + sz < pixels.size());
    auto beg_it { pixels.begin() + offset };
    for (uint16_t i { 0 }; i < sz; ++i) {
        pixels[offset + i].c = s[i];
    }
}

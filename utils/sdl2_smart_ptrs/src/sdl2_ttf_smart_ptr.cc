#include "sdl2_ttf_smart_ptr.hh"

namespace sdl2_smart_ptr {

namespace deleter {

void TtfFont::operator()(TTF_Font* fp) const { TTF_CloseFont(fp); }

}  // namespace deleter

auto make_unique(TTF_Font* tfp) {
    return unique::TtfFont{ tfp, deleter::TtfFont{} };
}

auto make_shared(TTF_Font* tfp) {
    static deleter::TtfFont dltr;
    return shared::TtfFont{ tfp, dltr };
}

}  // namespace sdl2_smart_ptr

#ifndef SDL2_TTF_SMART_PTR_HH
#define SDL2_TTF_SMART_PTR_HH

#include "SDL_ttf.h"     // TTF_Font

#include <memory>

namespace sdl2_smart_ptr {

namespace deleter {

struct TtfFont {
    void operator()(TTF_Font*) const;
};

}  // namespace deleter

namespace unique {

using TtfFont = std::unique_ptr<TTF_Font, deleter::TtfFont>;

}  // namespace unique

namespace shared {

using TtfFont = std::shared_ptr<TTF_Font>;

}  // namespace shared

namespace weak {

using TtfFont = std::weak_ptr<TTF_Font>;

}  // namespace weak

unique::TtfFont make_unique(TTF_Font*);

shared::TtfFont make_shared(TTF_Font*);

}  // namespace sdl2_smart_ptr


#endif  // SDL2_TTF_SMART_PTR_HH

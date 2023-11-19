#ifndef SDL2_RTF_SMART_PTR_HH
#define SDL2_RTF_SMART_PTR_HH

#include "SDL_rtf.h"     // RTF_Context

#include <memory>


namespace sdl2_smart_ptr {

namespace deleter {

struct RtfContext {
    void operator()(RTF_Context*) const;
};

}  // namespace deleter

namespace unique {

using RtfContext = std::unique_ptr<RTF_Context, deleter::RtfContext>;

}  // namespace unique

namespace shared {

using RtfContext = std::shared_ptr<RTF_Context>;

}  // namespace shared

namespace weak {

using RtfContext = std::weak_ptr<RTF_Context>;

}  // namespace weak

unique::RtfContext make_unique(RTF_Context*);

shared::RtfContext make_shared(RTF_Context*);

}  // namespace sdl2_smart_ptr


#endif  // SDL2_RTF_SMART_PTR_HH

#include "sdl2_rtf_smart_ptr.hh"

namespace sdl2_smart_ptr {

namespace deleter {

void RtfContext::operator()(RTF_Context* rcp) const { RTF_FreeContext(rcp); }

}  // namespace deleter

auto make_unique(RTF_Context* tfp) {
    return unique::RtfContext{ tfp, deleter::RtfContext{} };
}

auto make_shared(RTF_Context* tfp) {
    static deleter::RtfContext dltr;
    return shared::RtfContext{ tfp, dltr };
}

}  // namespace sdl2_smart_ptr

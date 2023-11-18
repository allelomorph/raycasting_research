#include "sdl2_mixer_smart_ptr.hh"

namespace sdl2_smart_ptr {

namespace deleter {

void MixChunk::operator()(Mix_Chunk* mcp) const { Mix_FreeChunk(mcp); }

void MixMusic::operator()(Mix_Music* mmp) const { Mix_FreeMusic(mmp); }

}  // namespace deleter

auto make_unique(Mix_Chunk* mcp) {
    return unique::MixChunk{ mcp, deleter::MixChunk{} };
}

auto make_unique(Mix_Music* mmp) {
    return unique::MixMusic{ mmp, deleter::MixMusic{} };
}

auto make_shared(Mix_Chunk* mcp) {
    static deleter::MixChunk dltr;
    return shared::MixChunk{ mcp, dltr };
}

auto make_shared(Mix_Music* mmp) {
    static deleter::MixMusic dltr;
    return shared::MixMusic{ mmp, dltr };
}

}  // namespace sdl2_smart_ptr

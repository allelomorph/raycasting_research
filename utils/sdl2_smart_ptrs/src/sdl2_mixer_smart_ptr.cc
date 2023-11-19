#include "sdl2_mixer_smart_ptr.hh"

namespace sdl2_smart_ptr {

namespace deleter {

void MixChunk::operator()(Mix_Chunk* mcp) const { Mix_FreeChunk(mcp); }

void MixMusic::operator()(Mix_Music* mmp) const { Mix_FreeMusic(mmp); }

}  // namespace deleter

unique::MixChunk make_unique(Mix_Chunk* mcp) {
    static const deleter::MixChunk dltr;
    return unique::MixChunk{ mcp, dltr };
}

unique::MixMusic make_unique(Mix_Music* mmp) {
    static const deleter::MixMusic dltr;
    return unique::MixMusic{ mmp, dltr };
}

shared::MixChunk make_shared(Mix_Chunk* mcp) {
    static const deleter::MixChunk dltr;
    return shared::MixChunk{ mcp, dltr };
}

shared::MixMusic make_shared(Mix_Music* mmp) {
    static const deleter::MixMusic dltr;
    return shared::MixMusic{ mmp, dltr };
}

}  // namespace sdl2_smart_ptr

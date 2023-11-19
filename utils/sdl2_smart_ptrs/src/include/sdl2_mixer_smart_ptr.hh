#ifndef SDL2_MIXER_SMART_PTR_HH
#define SDL2_MIXER_SMART_PTR_HH

#include "SDL_mixer.h"     // Mix_Chunk, Mix_Music

#include <memory>

namespace sdl2_smart_ptr {

namespace deleter {

struct MixChunk {
    void operator()(Mix_Chunk*) const;
};

struct MixMusic {
    void operator()(Mix_Music*) const;
};

}  // namespace deleter

namespace unique {

using MixChunk = std::unique_ptr<Mix_Chunk, deleter::MixChunk>;
using MixMusic = std::unique_ptr<Mix_Music, deleter::MixMusic>;

}  // namespace unique

namespace shared {

using MixChunk = std::shared_ptr<Mix_Chunk>;
using MixMusic = std::shared_ptr<Mix_Music>;

}  // namespace shared

namespace weak {

using MixChunk = std::weak_ptr<Mix_Chunk>;
using MixMusic = std::weak_ptr<Mix_Music>;

}  // namespace weak

unique::MixChunk make_unique(Mix_Chunk*);
unique::MixMusic make_unique(Mix_Music*);

shared::MixChunk make_shared(Mix_Chunk*);
shared::MixMusic make_shared(Mix_Music*);

}  // namespace sdl2_smart_ptr


#endif  // SDL2_MIXER_SMART_PTR_HH

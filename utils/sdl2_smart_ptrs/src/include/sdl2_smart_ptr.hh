#ifndef SDL2_SMART_PTR_HH
#define SDL2_SMART_PTR_HH

#include "SDL_mouse.h"     // SDL_Cursor
#include "SDL_mutex.h"     // SDL_cond SDL_mutex SDL_sem
#include "SDL_render.h"    // SDL_Renderer SDL_Texture
#include "SDL_surface.h"
#include "SDL_video.h"     // SDL_Window

#include <memory>

namespace sdl2_smart_ptr {

namespace deleter {

struct Cursor {
    void operator()(SDL_Cursor*) const;
};

struct CondVar {
    void operator()(SDL_cond*) const;
};

struct Mutex {
    void operator()(SDL_mutex*) const;
};

struct Renderer {
    void operator()(SDL_Renderer*) const;
};

struct Semaphore {
    void operator()(SDL_sem*) const;
};

struct Surface {
    void operator()(SDL_Surface*) const;
};

struct Texture {
    void operator()(SDL_Texture*) const;
};

struct Window {
    void operator()(SDL_Window*) const;
};

}  // namespace deleter

namespace unique {

using Cursor       = std::unique_ptr<SDL_Cursor,   deleter::Cursor>;
using CondVar      = std::unique_ptr<SDL_cond,     deleter::CondVar>;
using Mutex        = std::unique_ptr<SDL_mutex,    deleter::Mutex>;
using Renderer     = std::unique_ptr<SDL_Renderer, deleter::Renderer>;
using Semaphore    = std::unique_ptr<SDL_sem,      deleter::Semaphore>;
using Surface      = std::unique_ptr<SDL_Surface,  deleter::Surface>;
using Texture      = std::unique_ptr<SDL_Texture,  deleter::Texture>;
using Window       = std::unique_ptr<SDL_Window,   deleter::Window>;

}   // namespace unique

namespace shared {

using Cursor       = std::shared_ptr<SDL_Cursor>;
using CondVar      = std::shared_ptr<SDL_cond>;
using Mutex        = std::shared_ptr<SDL_mutex>;
using Renderer     = std::shared_ptr<SDL_Renderer>;
using Semaphore    = std::shared_ptr<SDL_sem>;
using Surface      = std::shared_ptr<SDL_Surface>;
using Texture      = std::shared_ptr<SDL_Texture>;
using Window       = std::shared_ptr<SDL_Window>;

}   // namespace shared

namespace weak {

using Cursor       = std::weak_ptr<SDL_Cursor>;
using CondVar      = std::weak_ptr<SDL_cond>;
using Mutex        = std::weak_ptr<SDL_mutex>;
using Renderer     = std::weak_ptr<SDL_Renderer>;
using Semaphore    = std::weak_ptr<SDL_sem>;
using Surface      = std::weak_ptr<SDL_Surface>;
using Texture      = std::weak_ptr<SDL_Texture>;
using Window       = std::weak_ptr<SDL_Window>;

}   // namespace weak

auto make_unique(SDL_Cursor*);
auto make_unique(SDL_cond*);
auto make_unique(SDL_mutex*);
auto make_unique(SDL_Renderer*);
auto make_unique(SDL_sem*);
auto make_unique(SDL_Surface*);
auto make_unique(SDL_Texture*);
auto make_unique(SDL_Window*);

auto make_shared(SDL_Cursor*);
auto make_shared(SDL_cond*);
auto make_shared(SDL_mutex*);
auto make_shared(SDL_Renderer*);
auto make_shared(SDL_sem*);
auto make_shared(SDL_Surface*);
auto make_shared(SDL_Texture*);
auto make_shared(SDL_Window*);

}  // namespace sdl2_smart_ptr


#endif  // SDL2_SMART_PTR_HH

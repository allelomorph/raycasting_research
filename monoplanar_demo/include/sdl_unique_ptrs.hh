#ifndef SDL_UNIQUE_PTRS_HH
#define SDL_UNIQUE_PTRS_HH

#include <SDL2/SDL_video.h>     // SDL_Window
#include <SDL2/SDL_render.h>    // SDL_Renderer SDL_Texture
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>       // TTF_Font

#include <memory>               // unique_ptr


namespace SdlDeleter {

struct Window {
    void operator()(SDL_Window* wp) const;
};

struct Renderer {
    void operator()(SDL_Renderer* rp) const;
};

struct Surface {
    void operator()(SDL_Surface* sp) const;
};

struct Texture {
    void operator()(SDL_Texture* tp) const;
};

struct TtfFont {
    void operator()(TTF_Font* fp) const;
};

}  // namespace SdlDeleter

using SdlWindowUnqPtr   = std::unique_ptr<SDL_Window,   SdlDeleter::Window>;
using SdlRendererUnqPtr = std::unique_ptr<SDL_Renderer, SdlDeleter::Renderer>;
using SdlSurfaceUnqPtr  = std::unique_ptr<SDL_Surface,  SdlDeleter::Surface>;
using SdlTextureUnqPtr  = std::unique_ptr<SDL_Texture,  SdlDeleter::Texture>;
using SdlTtfFontUnqPtr  = std::unique_ptr<TTF_Font,     SdlDeleter::TtfFont>;


#endif  // SDL_UNIQUE_PTRS_HH

#ifndef SDL_STRUCT_OSTREAM_OPS_HH
#define SDL_STRUCT_OSTREAM_OPS_HH


#include <map>
#include <vector>
#include <iostream>
#include <iomanip>

#include <cstdint>

#include <SDL2/SDL_render.h>     // SDL_RendererFlags SDL_TextureAccess SDL_Renderer SDL_Texture
#include <SDL2/SDL_pixels.h>     // SDL_PixelFormatEnum SDL_PixelFormat
#include <SDL2/SDL_video.h>      // SDL_WindowFlags SDL_Window
#include <SDL2/SDL_blendmode.h>  // SDL_BlendMode
#include <SDL2/SDL_surface.h>    // SDL_Surface
// #include <SDL2/SDL_ttf.h>       // TTF_Font


// SDL_RendererFlags - "Flags used when creating a rendering context"
const std::map<int, const char*> sdl_renderer_flag_names {
    { SDL_RENDERER_SOFTWARE,      "SDL_RENDERER_SOFTWARE" },      // 0x00000001
    // "The renderer is a software fallback"
    { SDL_RENDERER_ACCELERATED,   "SDL_RENDERER_ACCELERATED" },   // 0x00000002
    // "The renderer uses hardware acceleration"
    { SDL_RENDERER_PRESENTVSYNC,  "SDL_RENDERER_PRESENTVSYNC" },  // 0x00000004
    // "Present is synchronized with the refresh rate"
    { SDL_RENDERER_TARGETTEXTURE, "SDL_RENDERER_TARGETTEXTURE" }  // 0x00000008
    // "The renderer supports rendering to texture"
};

// SDL_PixelFormatEnum - use SDL_GetPixelFormatName()

// SDL_WindowFlags
const std::map<int, const char*> sdl_window_flag_names {
    { SDL_WINDOW_FULLSCREEN,         "SDL_WINDOW_FULLSCREEN" },          // 0x00000001
    // "fullscreen window"
    { SDL_WINDOW_OPENGL,             "SDL_WINDOW_OPENGL" },              // 0x00000002
    // "window usable with OpenGL context"
    { SDL_WINDOW_SHOWN,              "SDL_WINDOW_SHOWN" },               // 0x00000004
    // "window is visible"
    { SDL_WINDOW_HIDDEN,             "SDL_WINDOW_HIDDEN" },              // 0x00000008
    // "window is not visible"
    { SDL_WINDOW_BORDERLESS,         "SDL_WINDOW_BORDERLESS" },          // 0x00000010
    // "no window decoration"
    { SDL_WINDOW_RESIZABLE,          "SDL_WINDOW_RESIZABLE" },           // 0x00000020
    // "window can be resized"
    { SDL_WINDOW_MINIMIZED,          "SDL_WINDOW_MINIMIZED" },           // 0x00000040
    // "window is minimized"
    { SDL_WINDOW_MAXIMIZED,          "SDL_WINDOW_MAXIMIZED" },           // 0x00000080
    // "window is maximized"
    { SDL_WINDOW_INPUT_GRABBED,      "SDL_WINDOW_INPUT_GRABBED" },       // 0x00000100
    // "window has grabbed input focus"
    { SDL_WINDOW_INPUT_FOCUS,        "SDL_WINDOW_INPUT_FOCUS" },         // 0x00000200
    // "window has input focus"
    { SDL_WINDOW_MOUSE_FOCUS,        "SDL_WINDOW_MOUSE_FOCUS" },         // 0x00000400
    // "window has mouse focus"
    { SDL_WINDOW_FULLSCREEN_DESKTOP, "SDL_WINDOW_FULLSCREEN_DESKTOP" },  // ( SDL_WINDOW_FULLSCREEN | 0x00001000 )
    { SDL_WINDOW_FOREIGN,            "SDL_WINDOW_FOREIGN" },             // 0x00000800
    // "window not created by SDL"
    { SDL_WINDOW_ALLOW_HIGHDPI,      "SDL_WINDOW_ALLOW_HIGHDPI" },       // 0x00002000
    // "window should be created in high-DPI mode if supported. On macOS
    // NSHighResolutionCapable must be set true in the application's Info.plist
    // for this to have any effect."
    { SDL_WINDOW_MOUSE_CAPTURE,      "SDL_WINDOW_MOUSE_CAPTURE" },       // 0x00004000
    // "window has mouse captured (unrelated to INPUT_GRABBED)"
    { SDL_WINDOW_ALWAYS_ON_TOP,      "SDL_WINDOW_ALWAYS_ON_TOP" },       // 0x00008000
    // "window should always be above others"
    { SDL_WINDOW_SKIP_TASKBAR,       "SDL_WINDOW_SKIP_TASKBAR" },        // 0x00010000
    // "window should not be added to the taskbar"
    { SDL_WINDOW_UTILITY,            "SDL_WINDOW_UTILITY" },             // 0x00020000
    // "window should be treated as a utility window"
    { SDL_WINDOW_TOOLTIP,            "SDL_WINDOW_TOOLTIP" },             // 0x00040000
    // "window should be treated as a tooltip"
    { SDL_WINDOW_POPUP_MENU,         "SDL_WINDOW_POPUP_MENU" },          // 0x00080000
    // "window should be treated as a popup menu"
    { SDL_WINDOW_VULKAN,             "SDL_WINDOW_VULKAN" }               // 0x10000000
    // "window usable for Vulkan surface"
};

// SDL_TextureAccess - "The access pattern allowed for a texture."
const std::vector<const char*> sdl_texture_access_names {
    "SDL_TEXTUREACCESS_STATIC",     // "Changes rarely, not lockable"
    "SDL_TEXTUREACCESS_STREAMING",  // "Changes frequently, lockable"
    "SDL_TEXTUREACCESS_TARGET"      // "Texture can be used as a render target"
};

// SDL_BlendMode - "The blend mode used in SDL_RenderCopy() and drawing operations."
const std::map<int, const char*> sdl_blend_mode_names {
    { SDL_BLENDMODE_NONE,    "SDL_BLENDMODE_NONE" },    // 0x00000000
    // "no blending, dstRGBA = srcRGBA"
    { SDL_BLENDMODE_BLEND,   "SDL_BLENDMODE_BLEND" },   // 0x00000001
    // "alpha blending, dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA)),
    // dstA = srcA + (dstA * (1-srcA))"
    { SDL_BLENDMODE_ADD,     "SDL_BLENDMODE_ADD" },     // 0x00000002
    // "additive blending, dstRGB = (srcRGB * srcA) + dstRGB, dstA = dstA"
    { SDL_BLENDMODE_MOD,     "SDL_BLENDMODE_MOD" },     // 0x00000004
    // "color modulate, dstRGB = srcRGB * dstRGB, dstA = dstA"
    { SDL_BLENDMODE_INVALID, "SDL_BLENDMODE_INVALID" }  // 0x7FFFFFFF
};

std::ostream& operator<<(std::ostream& os, SDL_Renderer* rp) {
    if (rp == nullptr)
        return os << "(null)";

    SDL_RendererInfo info;
    SDL_GetRendererInfo(rp, &info);

    os << "{ " <<
        "name: \"" << info.name << "\", " <<
        "flags:";
    for (const auto& kv_pair : sdl_renderer_flag_names) {
        if (info.flags & kv_pair.first)
            os << ' ' << kv_pair.second;
    }
    os << ", ";
    for (uint32_t i { 0 }; i < info.num_texture_formats; ++i) {
        os << "texture_formats[" << i << "]: " <<
            SDL_GetPixelFormatName(info.texture_formats[i]) << ", ";
    }
    os << "max_texture_width:" << info.max_texture_width << ", " <<
        "max_texture_height:" << info.max_texture_height <<
       " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, SDL_PixelFormat* pfp) {
    if (pfp == nullptr)
        return os << "(null)";

    os << "{ " <<
        "palette: " << std::hex << pfp->palette << ", " <<
        "BitsPerPixel: " << std::dec << (int)(pfp->BitsPerPixel) << ", " <<
        "BytesPerPixel: " << (int)(pfp->BytesPerPixel) << ", " <<
        "Rmask: 0x" << std::hex << pfp->Rmask << ", " <<
        "Gmask: 0x" << pfp->Gmask << ", " <<
        "Bmask: 0x" << pfp->Bmask << ", " <<
        "Amask: 0x" << pfp->Amask <<
        " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, SDL_Surface* sp) {
    if (sp == nullptr)
        return os << "(null)";

    os << "{ " <<
        "format: " << sp->format << ", " <<
        "w: " << std::dec << sp->w << " h: " << sp->h << ", " <<
        "pitch: " << sp->pitch << ", " <<
        "pixels: 0x" << std::hex << sp->pixels << ", " <<
        "clip_rect: { " <<
        "w: " << std::dec << sp->clip_rect.w << ", " <<
        "h: " << sp->clip_rect.h << ", " <<
        "x: " << sp->clip_rect.x << ", " <<
        "y: " << sp->clip_rect.y << " }, " <<
        "refcount: " << sp->refcount <<
        " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, SDL_Window* wp) {
    if (wp == nullptr)
        return os << "(null)";

    uint32_t flags { SDL_GetWindowFlags(wp) };
    int w, h;
    SDL_GetWindowSize(wp, &w, &h);
    int min_w, min_h;
    SDL_GetWindowMinimumSize(wp, &min_w, &min_h);
    int max_w, max_h;
    SDL_GetWindowMaximumSize(wp, &max_w, &max_h);
    int x, y;
    SDL_GetWindowPosition(wp, &x, &y);
    float out_opacity;
    SDL_GetWindowOpacity(wp, &out_opacity);
    SDL_DisplayMode disp_mode;
    SDL_GetWindowDisplayMode(wp, &disp_mode);

    os << "{ " <<
        "id:" << SDL_GetWindowID(wp) << ", " <<
        "title: \"" << SDL_GetWindowTitle(wp) << "\", " <<
        "flags:";
    for (const auto& kv_pair : sdl_window_flag_names) {
        if (flags & kv_pair.first)
            os << ' ' << kv_pair.second;
    }
    os << ", ";
    os << "w: " << w << ", h: " << h << ", " <<
        "min_w: " << min_w << ", min_h: " << min_h << ", " <<
        "max_w: " << max_w << ", max_h: " << max_h << ", " <<
        "position: { x: " << x << ", y: " << y << " }, " <<
        "opacity: " << out_opacity << ", " <<
        "grabbing input: " << std::boolalpha << SDL_GetWindowGrab(wp) << ", ";
    os << "display: { index: " << SDL_GetWindowDisplayIndex(wp) << ", " <<
        "format: " << SDL_GetPixelFormatName(disp_mode.format) << ", " <<
        "w: " << disp_mode.w << ", " <<
        "h: " << disp_mode.h << ", " <<
        "refresh rate: " << disp_mode.refresh_rate << " }, ";
    os << "surface: " << SDL_GetWindowSurface(wp) << ", " <<
        "renderer: " << SDL_GetRenderer(wp) <<
        " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, SDL_Texture* tp) {
    if (tp == nullptr)
        return os << "(null)";

    uint32_t format;  // SDL_PixelFormatEnum
    int access;  // SDL_TextureAccess
    int w, h;
    SDL_QueryTexture(tp, &format, &access, &w, &h);
    SDL_BlendMode blend_mode;
    SDL_GetTextureBlendMode(tp, &blend_mode);
    uint8_t alpha;
    SDL_GetTextureAlphaMod(tp, &alpha);
    uint8_t r, g, b;
    SDL_GetTextureColorMod(tp, &r, &g, &b);

    os << "{ " <<
        "format: " << SDL_GetPixelFormatName(format) << ", " <<
        "access: " << sdl_texture_access_names.at(access) << ", " <<
        "w: " << w << ", h: " << h << ", " <<
        "blend mode: " << sdl_blend_mode_names.at(blend_mode) << ", " <<
        "alpha mod: " << (int)alpha << ", " <<
        "color mod: { r: " << (int)r << " g: " << (int)g << " b: " << (int)b << " }" << 
        " }";
    return os;
}

// std::ostream& operator<<(std::ostream& os, TTF_Font* fp) {
//     if (fp == nullptr)
//         return os << "(null)";
// }


#endif  // SDL_STRUCT_OSTREAM_OPS_HH

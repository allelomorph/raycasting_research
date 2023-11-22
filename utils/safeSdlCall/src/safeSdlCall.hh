#ifndef SAFESDLCALL_HH
#define SAFESDLCALL_HH


/*
 * Note: Most SDL2 extension *_GetError functions are macros for SDL_GetError:
 * ```
 * SDL_image.h:#define IMG_GetError    SDL_GetError
 * SDL_mixer.h:#define Mix_GetError    SDL_GetError
 * SDL_rtf.h:#define RTF_GetError    SDL_GetError
 * SDL_ttf.h:#define TTF_GetError    SDL_GetError
 * ```
 */
#include "SDL.h"       // SDL_GetError
// Except SDL_net, which defines its own wrapper for SDL_GetError
#include "SDL_net.h"   // SDLNet_GetError

#include <cassert>

#include <string_view>
#include <sstream>


/*
 * If performace becomes an issue an alias to a function pointer would work,
 *   but at the cost of being able to pass in functors. For comparison of
 *   std::function vs function pointers: https://stackoverflow.com/q/25848690
 * Note that when instantiating this type with a lambda, ReturnType must
 *   be passed as a template parameter to allow its deduction in safeSdlCall
 */
template<typename ReturnType>
class SdlRetTest : public std::function<bool(const ReturnType)> {};

/*
 * https://wiki.libsdl.org/SDL_GetError specifically forbids using the error
 *   string to determine error return, as several errors may have occured between
 *   the function call and SDL_GetError. This leaves us without a means to
 *   check functions that return void, but also can set errors, such as
 *   SDL_DestroyTexture. So a void return overload similar to safeLibcCall would
 *   be of no use here.
 */
template<typename FuncType, typename ReturnType, typename ...ParamTypes>
ReturnType safeSdlCall(FuncType&& sdl_func,
                       const std::string_view& sdl_func_name,
                       const SdlRetTest<ReturnType>& is_failure,
                       ParamTypes ...params) {
    /*
    assert(sdl_func_name.find("SDL_") == 0 ||
           sdl_func_name.find("IMG_") == 0 ||
           sdl_func_name.find("Mix_") == 0 ||
           sdl_func_name.find("SDLNet_") == 0 ||
           sdl_func_name.find("RTF_") == 0 ||
           sdl_func_name.find("TTF_") == 0);
    */
    ReturnType retval { sdl_func(params...) };
    if (is_failure(retval)) {
        std::ostringstream msg;
        msg << sdl_func_name << ": ";
        if (sdl_func_name.find("SDLNet_") == 0)
            msg << SDLNet_GetError();
        else
            msg << SDL_GetError();
        // log before throw in case exception is caught elsewhere
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", msg.str().c_str());
        throw std::runtime_error(msg.str());
    }
    return retval;
}


#endif  // SAFESDLCALL_HH

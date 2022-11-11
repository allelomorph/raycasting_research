#ifndef SAFESDLEXEC_HH
#define SAFESDLEXEC_HH


#include <SDL2/SDL.h>          // SDL_GetError

#include <cassert>

#include <string>
#include <sstream>


template <typename RetType>
using ReturnTest = bool (*)(RetType);

// Note: SDL2 extension *_GetError functions seem to all be macros for SDL_GetError:
// ```
// /usr/include/SDL2/SDL_ttf.h:284:#define TTF_GetError    SDL_GetError
// /usr/include/SDL2/SDL_image.h:153:#define IMG_GetError    SDL_GetError
// /usr/include/SDL2/SDL_mixer.h:640:#define Mix_GetError    SDL_GetError
// ```

// TBD: https://wiki.libsdl.org/SDL_GetError specifically forbids using the
//   error string to determine errors - so then how to check functions with
//   void return that set errors, such as SDL_DestroyTexture?

template<typename FuncPtrType, typename RetType, typename ...ParamTypes>
RetType safeSDLExec(FuncPtrType func, std::string func_name,
                    ReturnTest<RetType> is_failure_return,
                    ParamTypes ...params) {
    assert(func_name.find("SDL_") == 0 ||
           func_name.find("IMG_") == 0 ||
           func_name.find("Mix_") == 0 ||
           func_name.find("TTF_") == 0);
    RetType retval { func(params...) };
    if (is_failure_return(retval)) {
        std::ostringstream msg;
        msg << func_name << ": " << SDL_GetError();
        throw std::runtime_error(msg.str());
    }
    return retval;
}


#endif  // SAFESDLEXEC_HH

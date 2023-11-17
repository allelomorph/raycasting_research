#ifndef SAFESDLCALL_HH
#define SAFESDLCALL_HH


#include <SDL2/SDL.h>          // SDL_GetError

#include <cassert>

#include <string>
#include <sstream>


// Note: SDL2 extension *_GetError functions seem to all be macros for SDL_GetError:
// ```
// /usr/include/SDL2/SDL_ttf.h:284:#define TTF_GetError    SDL_GetError
// /usr/include/SDL2/SDL_image.h:153:#define IMG_GetError    SDL_GetError
// /usr/include/SDL2/SDL_mixer.h:640:#define Mix_GetError    SDL_GetError
// ```

// https://wiki.libsdl.org/SDL_GetError specifically forbids using the error
//   string to determine error return, as several errors may have occured between
//   the function call and SDL_GetError. This leaves us without a means to
//   check functions that return void, but also can set errors, such as
//   SDL_DestroyTexture. So a void return overload similar to safeCExec would
//   be of no use here.

// TBD: would prefer to pass is_failure_return as a templated type, to
//   accommodate lambdas, function pointers, and functors; but resolving
//   ReturnType from is_failure paramter type proved difficult in initial
//   attempts, see:
//   - https://stackoverflow.com/questions/8711855/get-lambda-parameter-type
//   - https://stackoverflow.com/questions/12202656/c11-lambda-implementation-and-memory-model

// TBD: how to allow for throwing other exception types, eg bad_alloc?

template<typename FuncPtrType, typename ReturnType, typename ...ParamTypes>
ReturnType safeSdlCall(FuncPtrType func, const std::string& func_name,
                       bool (*is_failure)(ReturnType),
                       ParamTypes ...params) {
    // TBD: this assert for every call may be too slow
    assert(func_name.find("SDL_") == 0 ||
           func_name.find("IMG_") == 0 ||
           func_name.find("Mix_") == 0 ||
           func_name.find("TTF_") == 0);
    ReturnType retval { func(params...) };
    if (is_failure(retval)) {
        std::ostringstream msg;
        msg << func_name << ": " << SDL_GetError();
        // log before throw in case exception is caught elsewhere
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", msg.str().c_str());
        throw std::runtime_error(msg.str());
    }
    return retval;
}


#define SDL_RETURN_TEST(ret_type, test) \
    static_cast<bool (*)(ret_type ret)>([](ret_type ret){ return test; })


#endif  // SAFESDLCALL_HH

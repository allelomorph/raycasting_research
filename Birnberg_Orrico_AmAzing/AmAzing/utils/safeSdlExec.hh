#ifndef SAFESDLEXEC_HH
#define SAFESDLEXEC_HH


#include <SDL2/SDL.h>          // SDL_GetError
#include <SDL2_ttf/SDL_ttf.h>  // TTF_GetError

#include <cassert>

#include <string>
#include <sstream>


template<typename FuncPtrType, typename RetType, typename ...ParamTypes>
RetType safeSdlExec(FuncPtrType func, std::string func_name,
                    RetType failure_retval, ParamTypes ...params) {
    assert(func_name.find("SDL_") == 0);
    RetType retval { func(params...) };
    if (retval == failure_retval) {
        std::ostringstream msg;
        msg << func_name << ": " << SDL_GetError();
        throw std::runtime_error(msg.str());
    }
    return retval;
}

// TBD: are there SDL_* functions that return void? If so, how to determine
//   fail state?

template<typename FuncPtrType, typename RetType, typename ...ParamTypes>
RetType safeTtfExec(FuncPtrType func, std::string func_name,
                    RetType failure_retval, ParamTypes ...params) {
    assert(func_name.find("TTF_") == 0);
    RetType retval { func(params...) };
    if (retval == failure_retval) {
        std::ostringstream msg;
        msg << func_name << ": " << TTF_GetError();
        throw std::runtime_error(msg.str());
    }
    return retval;
}

// TBD: are there TTF_* functions that return void? If so, how to determine
//   fail state?


#endif  // SAFESDLEXEC_HH

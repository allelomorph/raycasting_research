// errnoname.c uses C99 designated initializers and must be compiled separately
//   as C, then linked during C++ compilation
extern "C" {
#include "errnoname.h"
}

#include <string>
#include <sstream>

#include <cstring>    // strerror

template<typename FuncPtrType, typename RetType, typename ...ParamTypes>
RetType safeCExec(FuncPtrType func, std::string func_name,
                 RetType failure_retval, ParamTypes ...params) {
    errno = 0;
    RetType retval { func(params...) };
    if (retval == failure_retval) {
        std::ostringstream msg;
        msg << func_name << ": ";
        if (errno == 0)
            msg << "failure without setting errno";
        else
            msg << errnoname(errno) << " - " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
    return retval;
}

template<typename FuncPtrType, typename ...ParamTypes>
void safeCExecVoidRet(FuncPtrType func, std::string func_name,
                      ParamTypes ...params) {
    errno = 0;
    std::ostringstream msg;

    func(params...);
    if (errno != 0) {
        msg << func_name << ": ";
        msg << errnoname(errno) << " - " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
}

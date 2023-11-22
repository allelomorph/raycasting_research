#ifndef SAFELIBCCALL_HH
#define SAFELIBCCALL_HH


// errnoname.c uses C99 designated initializers and must be compiled separately
//   as C, then linked during C++ compilation
extern "C" {
#include "errnoname.h"
}

#include <cstring>    // strerror

#include <functional>
#include <stdexcept>  // runtime_error
#include <string_view>
#include <sstream>


/*
 * Using child classes instead of aliases due to need to differentiate testing
 *   only errno (int) vs only an int return value.
 * If performace becomes an issue similar aliases to function pointer types
 *   would work, but at the cost of making the errno/int return differentiation,
 *   and being able to pass in functors. For comparison of std::function vs
 *   function pointers: https://stackoverflow.com/q/25848690
 * Note that when instantiating these types with a lambda, ReturnType must
 *   be passed as a template parameter to allow its deduction in safeLibcCall
 */
template<typename ReturnType>
class LibcRetErrTest : public std::function<bool(const ReturnType, const int)> {};

template<typename ReturnType>
class LibcRetTest : public std::function<bool(const ReturnType)> {};

class LibcErrTest : public std::function<bool(const int)> {};


template<typename FuncType, typename ReturnType, typename ...ParamTypes>
ReturnType safeLibcCall(FuncType&& libc_func,
                        const std::string_view& libc_func_name,
                        const LibcRetErrTest<ReturnType>& is_failure,
                        ParamTypes ...params) {
    errno = 0;
    ReturnType retval { libc_func(params...) };
    if (is_failure(retval, errno)) {
        std::ostringstream msg;
        msg << libc_func_name << ": ";
        if (errno == 0)
            msg << "failure without setting errno";
        else
            msg << errnoname(errno) << " - " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
    return retval;
}

template<typename FuncType, typename ReturnType, typename ...ParamTypes>
ReturnType safeLibcCall(FuncType&& libc_func,
                        const std::string_view& libc_func_name,
                        const LibcRetTest<ReturnType>& is_failure,
                        ParamTypes ...params) {
    errno = 0;
    ReturnType retval { libc_func(params...) };
    if (is_failure(retval)) {
        std::ostringstream msg;
        msg << libc_func_name << ": ";
        if (errno == 0)
            msg << "failure without setting errno";
        else
            msg << errnoname(errno) << " - " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
    return retval;
}

template<typename FuncType, typename ...ParamTypes>
void safeLibcCall(FuncType&& libc_func,
                  const std::string_view& libc_func_name,
                  const LibcErrTest& is_failure,
                  ParamTypes ...params) {
    errno = 0;
    libc_func(params...);
    if (is_failure(errno)) {
        std::ostringstream msg;
        msg << libc_func_name << ": ";
        if (errno == 0)
            msg << "failure without setting errno";
        else
            msg << errnoname(errno) << " - " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
}

template<typename FuncType, typename ...ParamTypes>
void safeLibcCall(FuncType&& libc_func,
                  const std::string_view& libc_func_name,
                  ParamTypes ...params) {
    errno = 0;
    libc_func(params...);
    if (errno != 0) {
        std::ostringstream msg;
        msg << libc_func_name << ": " << errnoname(errno) <<
            " - " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
}


#endif  // SAFELIBCCALL_HH

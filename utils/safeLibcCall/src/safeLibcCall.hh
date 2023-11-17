#ifndef SAFELIBCCALL_HH
#define SAFELIBCCALL_HH


// errnoname.c uses C99 designated initializers and must be compiled separately
//   as C, then linked during C++ compilation
extern "C" {
#include "errnoname.h"
}

#include <cstring>    // strerror

#include <stdexcept>  // runtime_error
#include <string>
#include <sstream>


// TBD: how to allow for throwing other exception types, eg bad_alloc?

template<typename FuncPtrType, typename ReturnType, typename ...ParamTypes>
ReturnType safeLibcCall(FuncPtrType func, const std::string& func_name,
                     bool (*is_failure)(ReturnType, int), ParamTypes ...params) {
    errno = 0;
    ReturnType retval { func(params...) };
    if (is_failure(retval, errno)) {
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
void safeLibcCall(FuncPtrType func, const std::string& func_name,
               bool (*is_failure)(int), ParamTypes ...params) {
    errno = 0;
    func(params...);
    if (errno != 0 && is_failure(errno)) {
        std::ostringstream msg;
        msg << func_name << ": " <<
            errnoname(errno) << " - " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
}

template<typename FuncPtrType, typename ...ParamTypes>
void safeLibcCall(FuncPtrType func, const std::string& func_name,
               ParamTypes ...params) {
    errno = 0;
    func(params...);
    if (errno != 0) {
        std::ostringstream msg;
        msg << func_name << ": " <<
            errnoname(errno) << " - " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
}

#define C_RETURN_ERRNO_TEST(ret_type, test) \
    static_cast<bool (*)(ret_type ret, int err)>([](ret_type ret, int err){ return test; })
#define C_RETURN_TEST(ret_type, test) \
    static_cast<bool (*)(ret_type ret, int /*err*/)>([](ret_type ret, int /*err*/){ return test; })
#define C_ERRNO_TEST(test) \
    static_cast<bool (*)(int err)>([](int err){ return test; })


#endif  // SAFELIBCCALL_HH

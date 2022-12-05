// testing solution from https://stackoverflow.com/questions/8711855/get-lambda-parameter-type
//   to see if we can resolve safeSDLExec ReturnType from lambda/func ptr param
// see also: https://stackoverflow.com/questions/12202656/c11-lambda-implementation-and-memory-model

#include <iostream>
#include <type_traits>  // decay

#include "typeName.hh"


#include <SDL2/SDL.h>          // SDL_GetError

#include <cassert>

#include <string>
#include <sstream>

template<class FPtr>
struct function_traits;

template<class R, class C, class A1>
struct function_traits<R (C::*)(A1)>
{   // non-const specialization
    typedef A1 arg_type;
    typedef R result_type;
    typedef R type(A1);
};

template<class R, class C, class A1>
struct function_traits<R (C::*)(A1) const>
{   // const specialization
    typedef A1 arg_type;
    typedef R result_type;
    typedef R type(A1);
};

template<class R, class A1>
struct function_traits<R (*)(A1)>
{   // non-const specialization
    typedef A1 arg_type;
    typedef R result_type;
    typedef R type(A1);
};
/*
template<class R, class A1>
struct function_traits<R (*)(A1) const>
{   // const specialization
    typedef A1 arg_type;
    typedef R result_type;
    typedef R type(A1);
};
*/
/*
bool isFunctionPointer() { return true; };

template <typename FuncType, typename = void>
struct IsFailureTest : public std::false_type
{};
*/
template <typename TestType, typename = void>
struct IsFailureTest : public std::false_type
{};
/*
template <typename ReturnType, typename ...ParamTypes>
struct IsFailureTest<ReturnType (*f)(ParamTypes ...params)> : public std::true_type
{};
*/
template <typename ReturnType>
struct IsFailureTest<bool (*)(ReturnType)> : public std::true_type
{};

// for safeCExec with errno
template <typename ReturnType>
struct IsFailureTest<bool (*)(ReturnType, int)> : public std::true_type
{};
/*
template <typename FuncType, typename ReturnType, typename ...ParamTypes>
struct IsFailureTest<
    FuncType, std::void_t<decltype(&FuncType::operator())>> : public std::integral_constant<
                                   bool,
                                   std::is_convertible<ReturnType (*f)(ParamTypes ...params)>::value>
{};
*/
template <typename TestType, typename ReturnType>
struct IsFailureTest<TestType,
                     std::void_t<decltype(std::declval<TestType&>().operator(ReturnType))>> : public std::true_type
{};
/*
template <typename TestType, typename ReturnType>
struct IsFailureTest<TestType,
                     std::void_t<decltype(&TestType::operator(ReturnType, int))>> : public std::true_type
{}
*/
// Note: SDL2 extension *_GetError functions seem to all be macros for SDL_GetError:
// ```
// /usr/include/SDL2/SDL_ttf.h:284:#define TTF_GetError    SDL_GetError
// /usr/include/SDL2/SDL_image.h:153:#define IMG_GetError    SDL_GetError
// /usr/include/SDL2/SDL_mixer.h:640:#define Mix_GetError    SDL_GetError
// ```

// TBD: https://wiki.libsdl.org/SDL_GetError specifically forbids using the
//   error string to determine errors - so then how to check functions with
//   void return that set errors, such as SDL_DestroyTexture?

// TBD: would prefer to pass is_failure_return as a templated type, to
//   accommodate lambdas, function pointers, and functors; but resolving
//   ReturnType from is_failure_return paramter type proved difficult in
//   initial attempts, see:
//   - https://stackoverflow.com/questions/8711855/get-lambda-parameter-type
//   - https://stackoverflow.com/questions/12202656/c11-lambda-implementation-and-memory-model

template<typename FuncPtrType, typename ReturnType, typename TestType, typename ...ParamTypes>
ReturnType safeSDLExec(FuncPtrType func, const std::string func_name,
                       std::enable_if<IsFailureTest<TestType>::value, TestType> is_failure,
                       ParamTypes ...params) {
    assert(func_name.find("SDL_") == 0 ||
           func_name.find("IMG_") == 0 ||
           func_name.find("Mix_") == 0 ||
           func_name.find("TTF_") == 0);
    ReturnType retval { func(params...) };
    if (is_failure(retval)) {
        std::ostringstream msg;
        msg << func_name << ": " << SDL_GetError();
        throw std::runtime_error(msg.str());
    }
    return retval;
}


bool f(int i) {
    return i < 1;
}

int main() {
    auto lam { [](int ret){ return (ret != 0); } };
    struct function_traits<decltype(&decltype(lam)::operator())> ft {};
    std::cout << typeName<typename decltype(ft)::arg_type>() << '\n';

    struct function_traits<decltype(&f)> ft2 {};
    std::cout << typeName<typename decltype(ft2)::arg_type>() << '\n';
/*
    auto lam2 { [](int ret){ return (ret != 0); } };
    std::cout << typeName<std::decay<decltype(lam2)>::type>() << '\n';
    bool (*fp)(int) = lam2;
    std::cout << typeName<decltype(fp)>() << '\n';
*/
    safeSDLExec(SDL_Init, "SDL_Init",
                [](int ret){ return (ret != 0); },
                0);
    SDL_Quit();
}

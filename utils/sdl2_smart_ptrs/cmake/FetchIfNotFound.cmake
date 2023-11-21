# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
cmake_minimum_required(VERSION 3.16)

option(SKIP_FIND_BEFORE_FETCH
  "No attempt is made to find packages, instead all are handled with FetchContent"
  OFF)

# find_package integration in FetchContent requires cmake 3.24:
#   https://cmake.org/cmake/help/v3.27/guide/using-dependencies/index.html#fetchcontent-and-find-package-integration
# fetch_if_not_found roughly emulates this behavior for lower versions
#   of cmake
# !!! This relies on scripts used by find_package creating INTERFACE library
#   targets with the same names (including namespaces, if any,) as ALIAS targets
#   created by FetchContent
macro(fetch_if_not_found
    PKG_NAME    # expects string: package name
    FP_OPTIONS  # expects list:   find_package args beyond first
    FC_OPTIONS  # expects list:   FetchContent_Declare(ExternalProject_Add)
                #                   args beyond first
                )
  if(NOT SKIP_FIND_BEFORE_FETCH)
    find_package(${PKG_NAME} ${FP_OPTIONS})
  endif()
  if(NOT ${PKG_NAME}_FOUND)
    include(FetchContent)
    FetchContent_Declare(${PKG_NAME} ${FC_OPTIONS})
    FetchContent_MakeAvailable(${PKG_NAME})
  endif()
  # presence of package can now be tested by:
  #   `if(<pkg_name>_FOUND OR <lowercase pkg_name>_POPULATED)`
endmacro()

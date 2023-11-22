# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14) #???

project(SafeSdlCall
  VERSION 1.5
  DESCRIPTION "Templated C++ wrapper for SDL2 function calls; throws failure \
    return values as exceptions"
  LANGUAGES CXX
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(InSourceBuildGuard)

add_subdirectory(src)

cmake_minimum_required(VERSION 3.16)

project(sdl_smart_ptrs
  VERSION 1.0
  DESCRIPTION "C++ smart pointer wrappers for structs allocated by SDL2 and \
its auxillary libraries"
  LANGUAGES CXX
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(InSourceBuildGuard)

include(GetSDL2)

add_subdirectory(src)

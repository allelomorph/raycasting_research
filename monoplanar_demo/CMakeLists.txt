# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14)

project(MonplanarRaycastingDemo
  VERSION 2.0
  DESCRIPTION "Demo of single-plane (Wolfenstein-esque) raycasting engine, \
    using either SDL2 or the Linux terminal for I/O"
  LANGUAGES C CXX
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(InSourceBuildGuard)

# TBD do we need some form of extern/ for outside dependencies?
include(GetSDL2)
include(GetsafeLibcCall)
include(GetsafeSdlCall)
include(Getxterm_ctrl_seqs)

add_subdirectory(src bin)

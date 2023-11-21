cmake_minimum_required(VERSION 3.16)

project(MonplanarRaycastingDemo
  VERSION 2.0
  DESCRIPTION "Demo of single-plane (Wolfenstein-esque) raycasting engine, \
using either SDL2 or the Linux terminal for I/O"
  LANGUAGES CXX
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(InSourceBuildGuard)

# TBD do we need some form of extern/ for outside dependencies?
include(GetSDL2)
include(GetsafeLibcCall)
include(GetsafeSdlCall)
include(Getsdl2_smart_ptrs)
include(Getxterm_ctrl_seqs)

#[[
foreach(lib
    SDL2 SDL2_image SDL2_mixer
    SDL2_net SDL2_ttf SDL2_rtf
    )
  string(TOLOWER "${lib}" lower_lib)
  message("${lib}_FOUND: ${${lib}_FOUND}")
  message("${lower_lib}_POPULATED: ${${lower_lib}_POPULATED}")
endforeach()
]]

add_subdirectory(src bin)

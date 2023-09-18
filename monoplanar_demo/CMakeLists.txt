# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14)

project(MonplanarRaycastingDemo
  LANGUAGES C CXX
  VERSION 1.0
  DESCRIPTION "Demo of single-plane (Wolfenstein-esque) raycasting engine, \
    using either SDL2 or the Linux terminal for I/O"
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

####
# Secure Dependencies
####

# cmake 3.16-supplied FindSDL*.cmake modules are for SDL1.x, _not_ SDL2!
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(LocatePackageOrFetchContent)

# SDL2 release installed by local package manager and used in testing, but
#   does not set targets SDL2::SDL2 and SDL2::SDL2main:
#   2.0.10 0e9560aea22818884921e5e5064953257bfe7fa7
# earliest SDL2 release to define targets SDL2::SDL2 and SDL2::SDL2main:
#   2.0.16 25f9ed87ff6947d9576fc9d79dee0784e638ac58
# SDL2 current release at time of writing:
#   2.28.3 8a5ba43d00252c6c8b33c9aa4f1048222955ab4d
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  # 2.0.16
  GIT_TAG        25f9ed87ff6947d9576fc9d79dee0784e638ac58
  )
LocatePackageOrFetchContent(SDL2 "${FP_OPTIONS}" "${FC_OPTIONS}")

# SDL2_image release installed by local package manager and used in testing,
#   but has no CMakeLists.txt:
#   2.0.5 ab2a9c602623193d61827ccd395302d92d90fc38 (required SDL ver unknown)
# earliest SDL2_image release with CMakeLists.txt:
#   2.6.0 7b3347c0d90d1f1e9cc4e06e145432697ca4e68f (requires SDL 2.0.9)
# SDL2_image current release at time of writing:
#   2.6.3 d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829 (requires SDL 2.0.9)
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  # 2.6.3 (requires SDL 2.0.9)
  GIT_TAG        d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829
  )
LocatePackageOrFetchContent(SDL2_image "${FP_OPTIONS}" "${FC_OPTIONS}")

# SDL2_ttf release installed by local package manager and used in testing,
#   but does not set target SDL2_ttf::SDL2_ttf:
#   2.0.15 f34e0a8e08efdb5e074731866cf8663f8c2c0baf (required SDL ver unknown)
# SDL2_ttf current release at time of writing:
#   2.20.2 89d1692fd8fe91a679bb943d377bfbd709b52c23 (requires SDL 2.0.10)
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  # 2.20.2 (requires SDL 2.0.10)
  GIT_TAG        89d1692fd8fe91a679bb943d377bfbd709b52c23
  )
LocatePackageOrFetchContent(SDL2_ttf "${FP_OPTIONS}" "${FC_OPTIONS}")

####
# Build Configuration
####

# add_compile_options called after FetchContent so that dependency compile
#   commands are not affected, could alternately use target_compile_options
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/W4 /bigobj)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# note: Debug/Release modes can be used with -DCMAKE_BUILD_TYPE=(Debug|Release)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og --coverage")
endif()

set(SRC_DIR     ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/)
# TBD seems brittle to use dirs outside project
set(UTILS_DIR   ${PROJECT_SOURCE_DIR}/../utils)
set(HEADER_DIRS
  ${INCLUDE_DIR}
  ${UTILS_DIR}
  )

# (MP for "monoplanar")
set(MP_FILE_STEMS
  App
  DdaRaycastEngine
  FpsCalc
  KbdInputMgr
  Layout
  LinuxKbdInputMgr
  TtyWindowMgr
  TtyPixelBuffer
  sdl_unique_ptrs
  SdlWindowMgr
  SdlKbdInputMgr
  Vector2d
)

set(MP_SRC "${MP_FILE_STEMS}")
list(TRANSFORM MP_SRC PREPEND "${SRC_DIR}/")
list(TRANSFORM MP_SRC APPEND ".cc")

set(MP_SRC_HEADERS "${MP_FILE_STEMS}")
list(TRANSFORM MP_SRC_HEADERS PREPEND "${INCLUDE_DIR}/")
list(TRANSFORM MP_SRC_HEADERS APPEND ".hh")

set(MP_HEADER_ONLY
  KeyState.hh
  Settings.hh
  WindowMgr.hh)
list(TRANSFORM MP_HEADER_ONLY PREPEND ${INCLUDE_DIR})

# TBD investigate differences in behavior on header only updates (eg `touch
#   errnoname.h`, where errnoname target not recompiled, but any binary
#   including safeCExec.hh (which includes errnoname.h) _is_ recompiled...)
# errnoname.c uses designated initializers, which requires C99+ and is not
#   compatible with the C++ standard
add_library(errnoname OBJECT
  ${UTILS_DIR}/errnoname.c
  )
set_target_properties(errnoname PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
  INCLUDE_DIRECTORIES ${UTILS_DIR}
  )

add_executable(monoplanar_demo
  ${MP_SRC}
  ${MP_HEADER_ONLY}
  ${UTILS_DIR}/Xterm.cc
  ${PROJECT_SOURCE_DIR}/main.cc
  )
set_target_properties(monoplanar_demo PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  INCLUDE_DIRECTORIES "${HEADER_DIRS}"
  )
target_link_libraries(monoplanar_demo
  errnoname
  SDL2::SDL2
  SDL2_image::SDL2_image
  SDL2_ttf::SDL2_ttf
  )

####
# Script Debug
####

include(TestTarget)
foreach(target
    SDL2
    SDL2main
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image
    SDL2_image::SDL2_image
    SDL2_ttf
    SDL2_ttf::SDL2_ttf)
  test_target(${target})
endforeach()

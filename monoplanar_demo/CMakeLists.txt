# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14)

project(MonplanarRaycastingDemo LANGUAGES CXX VERSION 2.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

####
# Handle Dependencies
####

# cmake 3.16-supplied FindSDL*.cmake modules are for SDL1.x, _not_ SDL2!
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(MakeList)
include(LocatePackageOrFetchContent)

# SDL2 release installed by local package manager and used in testing:
#   2.0.10 0e9560aea22818884921e5e5064953257bfe7fa7
# SDL2 current release at time of writing:
#   2.28.3 8a5ba43d00252c6c8b33c9aa4f1048222955ab4d
MakeList(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  # 2.0.10
  GIT_TAG        0e9560aea22818884921e5e5064953257bfe7fa7
  )
LocatePackageOrFetchContent(SDL2 "${FP_OPTIONS}" "${FC_OPTIONS}")

# SDL2_image release installed by local package manager and used in testing,
#   but has no CMakeLists.txt:
#   2.0.5 ab2a9c602623193d61827ccd395302d92d90fc38 (required SDL ver unknown)
# earliest SDL2_image release with CMakeLists.txt:
#   2.6.0 7b3347c0d90d1f1e9cc4e06e145432697ca4e68f (requires SDL 2.0.9)
# SDL2_image current release at time of writing:
#   2.6.3 d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829 (requires SDL 2.0.9)
MakeList(FC_OPTIONS
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
MakeList(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  # 2.20.2 (requires SDL 2.0.10)
  GIT_TAG        89d1692fd8fe91a679bb943d377bfbd709b52c23
  )
LocatePackageOrFetchContent(SDL2_ttf "${FP_OPTIONS}" "${FC_OPTIONS}")

####
# Compile Options
####

# add_compile_options called after FetchContent so that dependency compile
#   commands are not affected, alternately use:
# ```
#target_compile_options(<target> [BEFORE]
#  <INTERFACE|PUBLIC|PRIVATE> [items1...]
#  [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
# ```
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

####
# Script Debug
####

#message("BUILDSYSTEM_TARGETS: ${BUILDSYSTEM_TARGETS}")
# requires cmake 3.21
# message("IMPORTED_TARGETS: ${IMPORTED_TARGETS}")
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

####
# Testing
####

# no find modules for these two with v3.16
find_program(CMAKE_CXX_CPPCHECK cppcheck)
find_program(CMAKE_CXX_CPPLINT cpplint)
# find_program(REQUIRED) not until v3.18, so we need to check for success manually
if (NOT CMAKE_CXX_CPPCHECK)
  message(FATAL_ERROR "Could not find cppcheck installed on system")
endif()
if (NOT CMAKE_CXX_CPPLINT)
  message(FATAL_ERROR "Could not find cpplint installed on system")
endif()

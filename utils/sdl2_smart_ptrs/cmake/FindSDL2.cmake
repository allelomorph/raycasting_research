# stripped down from:
#   https://github.com/aminosbh/sdl2-cmake-modules/blob/f3223e2b723d7984ee722c27c949b42e5bef3204/FindSDL2.cmake

# TBD add docs
# About use of of Restructured Text/`.rst` in cmake module docs:
#   https://github.com/Kitware/CMake/blob/master/Help/dev/documentation.rst#modules

#[=======================================================================[.rst:

#]=======================================================================]

# LINUX, BSD variables not available until cmake v3.25
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(WARNING
    "SDL2 detection not currently cross-platform for non-Linux systems")
endif()

# clearing cache variables before calls to find_path or find_library ensures
#   that they search every time the script is run, see:
#   https://cmake.org/cmake/help/v3.16/command/find_path.html
#unset(SDL2_INCLUDE_DIR CACHE)
find_path(SDL2_INCLUDE_DIR
  SDL.h
  PATH_SUFFIXES
    SDL2 include/SDL2 include
  DOC "Path to directory containing SDL2 headers"
)
list(APPEND SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}")

if (SDL2_INCLUDE_DIR)
  # set SDL2_VERSION
  include(SdlVersionFromHeader)
  sdl_version_from_header(SDL_version.h)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

find_library(SDL2_LIBRARY
  SDL2
  PATH_SUFFIXES
    lib
    ${VC_LIB_PATH_SUFFIX}
  DOC "Path to directory containing SDL2 libraries"
  )
list(APPEND SDL2_LIBRARIES "${SDL2_LIBRARY}")
find_library(SDL2MAIN_LIBRARY
  SDL2main
  PATH_SUFFIXES
    lib
    ${VC_LIB_PATH_SUFFIX}
  DOC "Path to directory containing SDL2main library"
  )
list(APPEND SDL2_LIBRARIES "${SDL2MAIN_LIBRARY}")

unset(VC_LIB_PATH_SUFFIX)

# Currently find_package_handle_standard_args (fphsa) is called for both SDL2
#   and SDL2main, but since this module is run by calling find_package(SDL2),
#   there are no SDL2main_FIND_* vars set. This can cause fphsa to ignore
#   version constraints on SDL2main that were meant for both it and SDL2.
# Relevant list of variables set by find_package from:
#   https://cmake.org/cmake/help/v3.16/command/find_package.html#package-file-interface-variables
# Only REQUIRED, QUIET, and VERSION (not COMPONENTS) find variables handled by
#   find_package_handle_standard_args, see:
#   https://cmake.org/cmake/help/v3.16/module/FindPackageHandleStandardArgs.html#command:find_package_handle_standard_args
foreach(find_var_suffix
    "_FIND_REQUIRED"
    "_FIND_QUIETLY"
    "_FIND_VERSION"
    "_FIND_VERSION_MAJOR"
    "_FIND_VERSION_MINOR"
    "_FIND_VERSION_PATCH"
    "_FIND_VERSION_TWEAK"
    "_FIND_VERSION_COUNT"
    "_FIND_VERSION_EXACT")
  if (DEFINED SDL2${find_var_suffix})  # ${CMAKE_FIND_PACKAGE_NAME} == "SDL2"
    set(SDL2main${find_var_suffix} "${SDL2${find_var_suffix}}")
  endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
  REQUIRED_VARS
    SDL2_LIBRARY
    SDL2_INCLUDE_DIR
  VERSION_VAR
    SDL2_VERSION
    )
find_package_handle_standard_args(SDL2main
  REQUIRED_VARS
    SDL2MAIN_LIBRARY
    SDL2_INCLUDE_DIR
  VERSION_VAR
    SDL2_VERSION
    )

# SDL2 dependencies should have already been checked in cases such as
#   installation by a package manager or building from source using cmake
#   on the target system. Dependencies checked again here only as a precaution
#   for copies of the library built elsewhere.
find_library (M_LIBRARY NAMES m REQUIRED)
unset(M_LIBRARY CACHE)
find_library (DL_LIBRARY NAMES dl REQUIRED)
unset(DL_LIBRARY CACHE)
# need for threads can be platform-specific
find_package(Threads QUIET REQUIRED)

# TBD garnish with results of pkg-config, eg -D_REENTRANT?
# SDL2::SDL2 matches target name if built from source with FetchContent
if(SDL2_FOUND AND
    NOT TARGET SDL2::SDL2)
  add_library(SDL2::SDL2 UNKNOWN IMPORTED)
  set_target_properties(SDL2::SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
    )
endif()

# SDL2::SDL2main matches target name if built from source with FetchContent
if(SDL2main_FOUND AND
    NOT TARGET SDL2::SDL2main)
  add_library(SDL2::SDL2main UNKNOWN IMPORTED)
  set_target_properties(SDL2::SDL2main PROPERTIES
    IMPORTED_LOCATION "${SDL2MAIN_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES SDL2::SDL2
    )
endif()

# hide these variables from cmake GUI
mark_as_advanced(SDL2_LIBRARY
                 SDL2MAIN_LIBRARY
                 SDL2_INCLUDE_DIR
                 )

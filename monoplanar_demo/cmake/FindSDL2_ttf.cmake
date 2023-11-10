# stripped down from:
#   /usr/share/cmake-3.16/Modules/FindSDL_ttf.cmake
#   https://github.com/aminosbh/sdl2-cmake-modules/blob/f3223e2b723d7984ee722c27c949b42e5bef3204/FindSDL2_ttf.cmake

# TBD add docs
# About use of of Restructured Text/`.rst` in cmake module docs:
#   https://github.com/Kitware/CMake/blob/master/Help/dev/documentation.rst#modules

#[=======================================================================[.rst:

#]=======================================================================]

# LINUX, BSD variables not available until cmake v3.25
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(WARNING
    "SDL2_ttf detection not currently cross-platform for non-Linux systems")
endif()

# https://cmake.org/cmake/help/latest/command/find_package.html#package-file-interface-variables
if(NOT SDL2_FOUND AND
    NOT sdl2_POPULATED AND
    NOT SDL2_ttf_FIND_QUIETLY)
    message(WARNING
      "Could not detect SDL2, which is required by SDL2_ttf")
endif()

# Search for the SDL2_ttf include directory
#unset(SDL2_TTF_INCLUDE_DIR CACHE)
find_path(SDL2_TTF_INCLUDE_DIR
  SDL_ttf.h
  PATH_SUFFIXES
    SDL2
    include/SDL2
    include
  DOC "Path to directory containing SDL2_ttf headers"
)

# Search for the SDL2_ttf library
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()
#unset(SDL2_TTF_LIBRARY CACHE)
find_library(SDL2_TTF_LIBRARY
  SDL2_ttf
  PATH_SUFFIXES
    lib
    ${VC_LIB_PATH_SUFFIX}
  DOC "Path to directory containing SDL2_ttf library"
)
unset(VC_LIB_PATH_SUFFIX)

# set SDL2_TTF_VERSION
include(SDLVersionStringFromHeader)
SDL_version_string_from_header(SDL2_TTF SDL_TTF SDL_ttf.h)

set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_ttf
  REQUIRED_VARS
    SDL2_TTF_LIBRARIES
    SDL2_TTF_INCLUDE_DIRS
  VERSION_VAR
    SDL2_TTF_VERSION
  )

# SDL2_ttf dependencies should have already been checked in cases such as
#   installation by a package manager or building from source using cmake
#   on the target system. Dependencies checked again here only as a precaution
#   for copies of the library built elsewhere.
find_package(Freetype QUIET REQUIRED)


# TBD garnish with results of pkg-config, eg -D_REENTRANT
# SDL2_ttf::SDL2_ttf matches target name if built from source with FetchContent
# https://github.com/libsdl-org/SDL_ttf/blob/89d1692fd8fe91a679bb943d377bfbd709b52c23/CMakeLists.txt#L87
if(SDL2_ttf_FOUND AND
    NOT TARGET SDL2_ttf::SDL2_ttf)
  add_library(SDL2_ttf::SDL2_ttf UNKNOWN IMPORTED)
  set_target_properties(SDL2_ttf::SDL2_ttf PROPERTIES
    IMPORTED_LOCATION "${SDL2_TTF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_TTF_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES SDL2::SDL2
    )
endif()

mark_as_advanced(SDL2_TTF_LIBRARY
                 SDL2_TTF_INCLUDE_DIR)

# stripped down from:
#   /usr/share/cmake-3.16/Modules/FindSDL_image.cmake
#   https://github.com/aminosbh/sdl2-cmake-modules/blob/f3223e2b723d7984ee722c27c949b42e5bef3204/FindSDL2_image.cmake

# TBD add docs
# About use of of Restructured Text/`.rst` in cmake module docs:
#   https://github.com/Kitware/CMake/blob/master/Help/dev/documentation.rst#modules

#[=======================================================================[.rst:

#]=======================================================================]

# LINUX, BSD variables not available until cmake v3.25
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(WARNING
    "SDL2_image detection not currently cross-platform for non-Linux systems")
endif()

# https://cmake.org/cmake/help/latest/command/find_package.html#package-file-interface-variables
if(NOT SDL2_FOUND AND
    NOT sdl2_POPULATED AND
    NOT SDL2_image_FIND_QUIETLY)
    message(WARNING
      "Could not detect SDL2, which is required by SDL2_image")
endif()

# Search for the SDL2_image include directory
#unset(SDL2_IMAGE_INCLUDE_DIR CACHE)
find_path(SDL2_IMAGE_INCLUDE_DIR
  SDL_image.h
  PATH_SUFFIXES
    SDL2
    include/SDL2
    include
  DOC "Path to directory containing SDL2_image headers"
)

# Search for the SDL2_image library
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()
#unset(SDL2_IMAGE_LIBRARY CACHE)
find_library(SDL2_IMAGE_LIBRARY
  SDL2_image
  PATH_SUFFIXES
    lib
    ${VC_LIB_PATH_SUFFIX}
  DOC "Path to directory containing SDL2_image library"
)
unset(VC_LIB_PATH_SUFFIX)

# set SDL2_IMAGE_VERSION
include(SDLVersionStringFromHeader)
SDL_version_string_from_header(SDL2_IMAGE SDL_IMAGE SDL_image.h)

set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY})
set(SDL2_IMAGE_INCLUDE_DIRS ${SDL2_IMAGE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_image
  REQUIRED_VARS
    SDL2_IMAGE_LIBRARIES
    SDL2_IMAGE_INCLUDE_DIRS
  VERSION_VAR
    SDL2_IMAGE_VERSION
  )

# TBD garnish with results of pkg-config, eg -D_REENTRANT
# SDL2_image::SDL2_image matches target name if built from source with FetchContent
# https://github.com/libsdl-org/SDL_image/blob/152c2801d5dc2018647c0c368cfb33902725d851/CMakeLists.txt#L213
if(SDL2_image_FOUND AND
    NOT TARGET SDL2_image::SDL2_image)
  add_library(SDL2_image::SDL2_image UNKNOWN IMPORTED)
  set_target_properties(SDL2_image::SDL2_image PROPERTIES
    IMPORTED_LOCATION "${SDL2_IMAGE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_IMAGE_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES SDL2::SDL2
    )
endif()

mark_as_advanced(SDL2_IMAGE_LIBRARY
                 SDL2_IMAGE_INCLUDE_DIR)

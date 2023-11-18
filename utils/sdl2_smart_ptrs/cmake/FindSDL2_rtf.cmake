# TBD cmake version?

# TBD add docs
# About use of of Restructured Text/`.rst` in cmake module docs:
#   https://github.com/Kitware/CMake/blob/master/Help/dev/documentation.rst#modules

#[=======================================================================[.rst:

#]=======================================================================]

# LINUX, BSD variables not available until cmake v3.25
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(WARNING
    "SDL2_rtf detection not currently cross-platform for non-Linux systems")
endif()

# https://cmake.org/cmake/help/latest/command/find_package.html#package-file-interface-variables
if(NOT SDL2_FOUND AND
    NOT sdl2_POPULATED AND
    NOT SDL2_rtf_FIND_QUIETLY)
    message(WARNING
      "Could not detect SDL2, which is required by SDL2_rtf")
endif()

# Search for the SDL2_rtf include directory
find_path(SDL2_RTF_INCLUDE_DIR
  SDL_rtf.h
  PATH_SUFFIXES
    SDL2
    include/SDL2
    include
  DOC "Path to directory containing SDL2_rtf headers"
)

if (SDL2_RTF_INCLUDE_DIR)
  # set SDL2_RTF_VERSION
  include(SdlVersionFromHeader)
  sdl_version_from_header(SDL_rtf.h)
endif()

# Search for the SDL2_rtf library
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

find_library(SDL2_RTF_LIBRARY
  SDL2_rtf
  PATH_SUFFIXES
    lib
    ${VC_LIB_PATH_SUFFIX}
  DOC "Path to directory containing SDL2_rtf library"
)
unset(VC_LIB_PATH_SUFFIX)

set(SDL2_RTF_LIBRARIES ${SDL2_RTF_LIBRARY})
set(SDL2_RTF_INCLUDE_DIRS ${SDL2_RTF_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_rtf
  REQUIRED_VARS
    SDL2_RTF_LIBRARIES
    SDL2_RTF_INCLUDE_DIRS
  VERSION_VAR
    SDL2_RTF_VERSION
  )

# SDL2_rtf dependencies should have already been checked in cases such as
#   installation by a package manager or building from source using cmake
#   on the target system. Dependencies checked again here only as a precaution
#   for copies of the library built elsewhere.
# TBD

# TBD garnish with results of pkg-config, eg -D_REENTRANT
# SDL2_rtf::SDL2_rtf matches target name if built from source with FetchContent
if(SDL2_rtf_FOUND AND
    NOT TARGET SDL2_rtf::SDL2_rtf)
  add_library(SDL2_rtf::SDL2_rtf UNKNOWN IMPORTED)
  set_target_properties(SDL2_rtf::SDL2_rtf PROPERTIES
    IMPORTED_LOCATION "${SDL2_RTF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_RTF_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES SDL2::SDL2
    )
endif()

mark_as_advanced(SDL2_RTF_LIBRARY
                 SDL2_RTF_INCLUDE_DIR)

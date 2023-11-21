function(sdl_version_from_header
    header_file  # string eg SDL_image.h
    )
  set(var_prefix   "SDL2")
  set(macro_prefix "SDL")
  if (   header_file STREQUAL "SDL_image.h")
    string(APPEND var_prefix   "_IMAGE")
    string(APPEND macro_prefix "_IMAGE")
  elseif(header_file STREQUAL "SDL_mixer.h")
    string(APPEND var_prefix   "_MIXER")
    string(APPEND macro_prefix "_MIXER")
  elseif(header_file STREQUAL "SDL_net.h")
    string(APPEND var_prefix   "_NET")
    string(APPEND macro_prefix "_NET")
  elseif(header_file STREQUAL "SDL_rtf.h")
    string(APPEND var_prefix   "_RTF")
    string(APPEND macro_prefix "_RTF")
  elseif(header_file STREQUAL "SDL_ttf.h")
    string(APPEND var_prefix   "_TTF")
    string(APPEND macro_prefix "_TTF")
  endif()

  if(NOT ${var_prefix}_INCLUDE_DIR)
    message(WARNING "${var_prefix}_INCLUDE_DIR not defined, could not resolve \
${var_prefix}_VERSION")
    return()
  endif()
  if(NOT EXISTS "${${var_prefix}_INCLUDE_DIR}/${header_file}")
    message(WARNING "${${var_prefix}_INCLUDE_DIR}/${header_file} not found, \
could not resolve ${var_prefix}_VERSION")
    return()
  endif()

  file(STRINGS "${${var_prefix}_INCLUDE_DIR}/${header_file}"
    ${var_prefix}_VERSION_MAJOR_LINE REGEX
    "^#define[ \t]+${macro_prefix}_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${${var_prefix}_INCLUDE_DIR}/${header_file}"
    ${var_prefix}_VERSION_MINOR_LINE REGEX
    "^#define[ \t]+${macro_prefix}_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${${var_prefix}_INCLUDE_DIR}/${header_file}"
    ${var_prefix}_VERSION_PATCH_LINE REGEX
    "^#define[ \t]+${macro_prefix}_PATCHLEVEL[ \t]+[0-9]+$")

  string(REGEX REPLACE
    "^#define[ \t]+${macro_prefix}_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1"
    ${var_prefix}_VERSION_MAJOR "${${var_prefix}_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE
    "^#define[ \t]+${macro_prefix}_MINOR_VERSION[ \t]+([0-9]+)$" "\\1"
    ${var_prefix}_VERSION_MINOR "${${var_prefix}_VERSION_MINOR_LINE}")
  string(REGEX REPLACE
    "^#define[ \t]+${macro_prefix}_PATCHLEVEL[ \t]+([0-9]+)$" "\\1"
    ${var_prefix}_VERSION_PATCH "${${var_prefix}_VERSION_PATCH_LINE}")

  # matching naming conventions in find_package config mode, although when using
  #   a find module version is not checked, see:
  #   https://cmake.org/cmake/help/v3.16/command/find_package.html#version-selection
  set(${var_prefix}_VERSION
    "${${var_prefix}_VERSION_MAJOR}.${${var_prefix}_VERSION_MINOR}.${${var_prefix}_VERSION_PATCH}")
  message(STATUS "${var_prefix}_VERSION derived from ${${var_prefix}_INCLUDE_DIR}/${header_file} as ${${var_prefix}_VERSION}")
  set(${var_prefix}_VERSION "${${var_prefix}_VERSION}" PARENT_SCOPE)

endfunction()

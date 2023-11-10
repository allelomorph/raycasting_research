function(SDL_version_string_from_header
    VAR_PREFIX            # string eg SDL2_IMAGE
    MACRO_PREFIX          # string eg SDL_IMAGE
    HEADER_FILE           # string eg SDL_image.h
    )
  # strings ending in -NOTFOUND evaluate to FALSE
  if(NOT ${VAR_PREFIX}_INCLUDE_DIR OR
      NOT EXISTS "${${VAR_PREFIX}_INCLUDE_DIR}/${HEADER_FILE}")
    message(WARNING "Could not set ${VAR_PREFIX}_VERSION due to missing ${${VAR_PREFIX}_INCLUDE_DIR}/${HEADER_FILE}")
    return()
  endif()

  file(STRINGS "${${VAR_PREFIX}_INCLUDE_DIR}/${HEADER_FILE}"
    ${VAR_PREFIX}_VERSION_MAJOR_LINE REGEX
    "^#define[ \t]+${MACRO_PREFIX}_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${${VAR_PREFIX}_INCLUDE_DIR}/${HEADER_FILE}"
    ${VAR_PREFIX}_VERSION_MINOR_LINE REGEX
    "^#define[ \t]+${MACRO_PREFIX}_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${${VAR_PREFIX}_INCLUDE_DIR}/${HEADER_FILE}"
    ${VAR_PREFIX}_VERSION_PATCH_LINE REGEX
    "^#define[ \t]+${MACRO_PREFIX}_PATCHLEVEL[ \t]+[0-9]+$")

  string(REGEX REPLACE
    "^#define[ \t]+${MACRO_PREFIX}_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1"
    ${VAR_PREFIX}_VERSION_MAJOR "${${VAR_PREFIX}_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE
    "^#define[ \t]+${MACRO_PREFIX}_MINOR_VERSION[ \t]+([0-9]+)$" "\\1"
    ${VAR_PREFIX}_VERSION_MINOR "${${VAR_PREFIX}_VERSION_MINOR_LINE}")
  string(REGEX REPLACE
    "^#define[ \t]+${MACRO_PREFIX}_PATCHLEVEL[ \t]+([0-9]+)$" "\\1"
    ${VAR_PREFIX}_VERSION_PATCH "${${VAR_PREFIX}_VERSION_PATCH_LINE}")

  # matching naming conventions in find_package config mode, although when using
  #   a find module version is not checked, see:
  #   https://cmake.org/cmake/help/v3.16/command/find_package.html#version-selection
  set(${VAR_PREFIX}_VERSION
    ${${VAR_PREFIX}_VERSION_MAJOR}.${${VAR_PREFIX}_VERSION_MINOR}.${${VAR_PREFIX}_VERSION_PATCH}
    PARENT_SCOPE)

endfunction()

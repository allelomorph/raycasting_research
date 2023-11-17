cmake_minimum_required(VERSION 3.16)

# Functions adapted from https://stackoverflow.com/questions/32183975

function(get_cmake_property_list)
  # Get all propreties supported by current cmake version
  execute_process(COMMAND
    cmake --help-property-list OUTPUT_VARIABLE cmake_property_list)
  # Convert command output to list
  string(REGEX REPLACE ";" "\\\\;" cmake_property_list "${cmake_property_list}")
  string(REGEX REPLACE "\n" ";" cmake_property_list "${cmake_property_list}")

  # Populate "<CONFIG>" with available_config_types
  set(available_config_types ${CMAKE_CONFIGURATION_TYPES})
  list(APPEND available_config_types ${CMAKE_BUILD_TYPE})
  list(TRANSFORM available_config_types TOUPPER)
  set(config_lines ${cmake_property_list})
  list(FILTER config_lines INCLUDE REGEX "<CONFIG>")
  list(FILTER cmake_property_list EXCLUDE REGEX "<CONFIG>")
  foreach(config_line IN LISTS config_lines)
    foreach(config IN LISTS available_config_types)
      string(REPLACE "<CONFIG>" "${config}" new_config_line "${config_line}")
      list(APPEND cmake_property_list ${new_config_line})
    endforeach()
  endforeach()

  # Populate "<LANG>" with supported_langs
  # Hardcoded list of languages, see:
  #   - https://cmake.org/cmake/help/latest/command/project.html#options
  #   - https://stackoverflow.com/a/44477728/240845
  # These could be individually verified with check_language(<lang>)
  set(supported_langs
    ASM-ATT ASM ASM_MASM ASM_NASM
    C CSHARP CUDA CXX FORTRAN HIP ISPC
    JAVA OBJC OBJCXX RC SWIFT
    )
  set(lang_lines ${cmake_property_list})
  list(FILTER lang_lines INCLUDE REGEX "<LANG>")
  list(FILTER cmake_property_list EXCLUDE REGEX "<LANG>")
  foreach(lang_line IN LISTS lang_lines)
    foreach(lang IN LISTS supported_langs)
      string(REPLACE "<LANG>" "${lang}" new_lang_line "${lang_line}")
      list(APPEND cmake_property_list ${new_lang_line})
    endforeach()
  endforeach()

  # TBD Other target property wildcards ignored for now:
  # - LIBRARY       eg LINK_LIBRARY_OVERRIDE_<LIBRARY>
  # - NAME          eg CXX_MODULE_DIRS_<NAME>;CXX_MODULE_SET_<NAME>;
  #                    HEADER_DIRS_<NAME>;HEADER_SET_<NAME>
  # - an-attribute  eg XCODE_ATTRIBUTE_<an-attribute>
  # - refname       eg VS_DOTNET_REFERENCE_<refname>,
  #                    VS_DOTNET_REFERENCEPROP_<refname>_TAG_<tagname>
  # - tool          eg VS_SOURCE_SETTINGS_<tool>
  # - type          eg XCODE_EMBED_<type>;XCODE_EMBED_<type>_CODE_SIGN_ON_COPY;
  #                    XCODE_EMBED_<type>_PATH;XCODE_EMBED_<type>_REMOVE_HEADERS_ON_COPY
  # - variable      eg VS_GLOBAL_<variable>

  # Directly reading LOCATION property violates CMP0026, see:
  #   - https://stackoverflow.com/a/58244245
  #   - https://stackoverflow.com/q/32197663
  # If config-time reading of LOCATION is still only deprecated and not disabled,
  #   one could use `cmake_policy(PUSH) cmake_policy(SET CMP0026 OLD)` before
  #   and `cmake_policy(POP)` after calls to get_target_property, although this
  #   will emit warnings
  list(FILTER cmake_property_list EXCLUDE REGEX "^LOCATION$|^LOCATION_|_LOCATION$")
  list(REMOVE_DUPLICATES cmake_property_list)
  list(SORT cmake_property_list)

  # Attempting to get properties on INTERFACE targets that are not whitelisted
  #   for that purpose will result in warnings/errors, see:
  #   https://cmake.org/cmake/help/v3.18/manual/cmake-buildsystem.7.html#interface-libraries
  # cmake 3.19 and above may not make such a distinction, see:
  #   https://discourse.cmake.org/t/how-to-find-current-interface-library-property-whitelist/4784/2
  set(cmake_whitelisted_property_list ${cmake_property_list})
  list(FILTER cmake_whitelisted_property_list INCLUDE REGEX
    "^(COMPATIBLE_INTERFACE_(BOOL|NUMBER_MAX|NUMBER_MIN|STRING)|EXPORT_NAME|EXPORT_PROPERTIES|IMPORTED(|_LIBNAME_[_A-Z]*)|INTERFACE_[_A-Z]*|MANUALLY_ADDED_DEPENDENCIES|MAP_IMPORTED_CONFIG_[_A-Z]*|NAME|NO_SYSTEM_FROM_IMPORTED|TYPE)$")

  set(CMAKE_PROPERTY_LIST ${cmake_property_list} PARENT_SCOPE)
  set(CMAKE_WHITELISTED_PROPERTY_LIST ${cmake_whitelisted_property_list} PARENT_SCOPE)
endfunction(get_cmake_property_list)

get_cmake_property_list()

function(print_target_properties target)
  if(NOT TARGET ${target})
    return()
  endif()

  get_target_property(target_type ${target} TYPE)
  if(target_type STREQUAL "INTERFACE_LIBRARY")
    set(properties ${CMAKE_WHITELISTED_PROPERTY_LIST})
  else()
    set(properties ${CMAKE_PROPERTY_LIST})
  endif()

  message ("Properties of target ${target}:")
  foreach (property ${properties})
    get_target_property(value ${target} ${property})
    if(NOT value MATCHES "NOTFOUND$")
      message("    ${property}: ${value}")
    endif()
  endforeach(property)
endfunction(print_target_properties)

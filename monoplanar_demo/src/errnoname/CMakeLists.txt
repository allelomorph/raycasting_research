# TBD investigate differences in behavior on header only updates (eg `touch
#   errnoname.h`, where errnoname target not recompiled, but any binary
#   including safeCExec.hh (which includes errnoname.h) _is_ recompiled...)
# errnoname.c uses designated initializers, which requires C99+ and is not
#   compatible with the C++ standard
add_library(errnoname_obj OBJECT
  errnoname.c
  )
set_target_properties(errnoname_obj PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
  POSITION_INDEPENDENT_CODE ON
  # PUBLIC_HEADER only needed if installing
  PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/errnoname.h"
  )
include(SetStrictCompileOptions)
set_strict_compile_options(errnoname_obj)
target_include_directories(errnoname_obj PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )

add_library(errnoname_static STATIC)
target_link_libraries(errnoname_static errnoname_obj)
# TBD is this redundant? Why is it not inherited from errnoname_obj?
target_include_directories(errnoname_static INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
set_target_properties(errnoname_static PROPERTIES
  ARCHIVE_OUTPUT_NAME errnoname
  )

add_library(errnoname_shared SHARED)
target_link_libraries(errnoname_shared errnoname_obj)
# TBD is this redundant? Why is it not inherited from errnoname_obj?
target_include_directories(errnoname_shared INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
set_target_properties(errnoname_shared PROPERTIES
  LIBRARY_OUTPUT_NAME errnoname
  )

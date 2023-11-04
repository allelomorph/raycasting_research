# TBD cmake version?

# ??? notice how here targets are made available in parent scope, but not when
#   using add_subdirectory in the root list file with extern/
add_subdirectory(errnoname)

# TBD seems brittle to use dirs outside project
set(UTILS_DIR   ${PROJECT_SOURCE_DIR}/../utils)

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
list(TRANSFORM MP_SRC APPEND ".cc")

set(MP_SRC_HEADERS "${MP_FILE_STEMS}")
list(TRANSFORM MP_SRC_HEADERS APPEND ".hh")

# TBD add repcompilation?
set(MP_HEADER_ONLY
  KeyState.hh
  Settings.hh
  WindowMgr.hh)

add_executable(monoplanar_demo
  ${MP_SRC}
  ${UTILS_DIR}/Xterm.cc
  main.cc
  )
set_target_properties(monoplanar_demo PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  )
include(SetStrictCompileOptions)
set_strict_compile_options(monoplanar_demo)
target_include_directories(monoplanar_demo PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${UTILS_DIR}"
  )
target_link_libraries(monoplanar_demo
  errnoname_shared
  SDL2::SDL2
  SDL2_image::SDL2_image
  SDL2_ttf::SDL2_ttf
  )

# note: Debug/Release modes can be used with -DCMAKE_BUILD_TYPE=(Debug|Release)
#    list(APPEND CMAKE_CXX_FLAGS_DEBUG
#      -Og --coverage

####
# Script Debug
####

message("testing targets in src")

include(TestTarget)
foreach(target
    SDL2
    SDL2main
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image
    SDL2_image::SDL2_image
    SDL2_ttf
    SDL2_ttf::SDL2_ttf
    errnoname_obj
    errnoname_static
    errnoname_shared)
  test_target(${target})
endforeach()

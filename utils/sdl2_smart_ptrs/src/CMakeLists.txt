cmake_minimum_required(VERSION 3.16)


add_library(sdl2_smart_ptr_obj OBJECT
  sdl2_smart_ptr.cc
  sdl2_mixer_smart_ptr.cc
  sdl2_net_smart_ptr.cc
  sdl2_rtf_smart_ptr.cc
  sdl2_ttf_smart_ptr.cc
  )
set_target_properties(sdl2_smart_ptr_obj PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  POSITION_INDEPENDENT_CODE ON
  )
include(SetStrictCompileOptions)
set_strict_compile_options(sdl2_smart_ptr_obj)
target_include_directories(sdl2_smart_ptr_obj PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
target_link_libraries(sdl2_smart_ptr_obj
  SDL2::SDL2
  SDL2_mixer::SDL2_mixer
  SDL2_net::SDL2_net
  SDL2_rtf::SDL2_rtf
  SDL2_ttf::SDL2_ttf
  )

add_library(sdl2_smart_ptr_static STATIC)
target_link_libraries(sdl2_smart_ptr_static sdl2_smart_ptr_obj)
target_include_directories(sdl2_smart_ptr_static INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
set_target_properties(sdl2_smart_ptr_static PROPERTIES
  ARCHIVE_OUTPUT_NAME sdl2_smart_ptr
  )

add_library(sdl2_smart_ptr_shared SHARED)
target_link_libraries(sdl2_smart_ptr_shared sdl2_smart_ptr_obj)
target_include_directories(sdl2_smart_ptr_shared INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
set_target_properties(sdl2_smart_ptr_shared PROPERTIES
  LIBRARY_OUTPUT_NAME sdl2_smart_ptr
  )

#[[
include(PrintTargetProperties)
foreach(target
    SDL2
    SDL2main
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image
    SDL2_image::SDL2_image
    SDL2_mixer
    SDL2_mixer::SDL2_mixer
    SDL2_net
    SDL2_net::SDL2_net
    SDL2_rtf
    SDL2_rtf::SDL2_rtf
    SDL2_ttf
    SDL2_ttf::SDL2_ttf
    sdl2_smart_ptr_object
    sdl2_smart_ptr_static
    sdl2_smart_ptr_shared
    )
  print_target_properties(${target})
endforeach()
]]

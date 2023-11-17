# version?

add_library(safeSdlCall INTERFACE)
target_include_directories(safeSdlCall INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )
target_precompile_headers(safeSdlCall INTERFACE
  safeSdlCall.hh
  )
target_link_libraries(safeSdlCall INTERFACE
  SDL2::SDL2
  )

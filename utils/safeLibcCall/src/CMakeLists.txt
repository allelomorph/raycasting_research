# version?

add_subdirectory(errnoname)

add_library(safeLibcCall INTERFACE)
target_include_directories(safeLibcCall INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )
target_precompile_headers(safeLibcCall INTERFACE
  safeLibcCall.hh
  )
target_link_libraries(safeLibcCall INTERFACE
  errnoname_shared
  )

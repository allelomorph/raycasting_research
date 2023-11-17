# version?

add_library(xterm_ctrl_seqs_obj OBJECT
  xterm_ctrl_seqs.cc
  )
set_target_properties(xterm_ctrl_seqs_obj PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  POSITION_INDEPENDENT_CODE ON
  )
include(SetStrictCompileOptions)
set_strict_compile_options(xterm_ctrl_seqs_obj)
target_include_directories(xterm_ctrl_seqs_obj PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )

add_library(xterm_ctrl_seqs_static STATIC)
target_link_libraries(xterm_ctrl_seqs_static xterm_ctrl_seqs_obj)
target_include_directories(xterm_ctrl_seqs_static INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
set_target_properties(xterm_ctrl_seqs_static PROPERTIES
  ARCHIVE_OUTPUT_NAME xterm_ctrl_seqs
  )

add_library(xterm_ctrl_seqs_shared SHARED)
target_link_libraries(xterm_ctrl_seqs_shared xterm_ctrl_seqs_obj)
target_include_directories(xterm_ctrl_seqs_shared INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
set_target_properties(xterm_ctrl_seqs_shared PROPERTIES
  LIBRARY_OUTPUT_NAME xterm_ctrl_seqs
  )

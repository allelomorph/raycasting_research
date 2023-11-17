# newest features used: TBD
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.16) #???

project(xterm_ctrl_seqs
  VERSION 1.0
  DESCRIPTION "Output streamable classes designed to generate Linux xterm \
    control sequence initiator (CSI) codes for formatting text output"
  LANGUAGES CXX
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(InSourceBuildGuard)

add_subdirectory(src)

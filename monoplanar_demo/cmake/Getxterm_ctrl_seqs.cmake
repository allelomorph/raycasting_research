# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14)

# TBD if using LocatePackageOrFetchContent, need to bypass find_package or
#   create find module for xterm_ctrl_seqs
#[[
include(LocatePackageOrFetchContent)
set(FC_OPTIONS
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/../utils/xterm_ctrl_seqs/"
  )
LocatePackageOrFetchContent(xterm_ctrl_seqs "" "${FC_OPTIONS}")
]]

include(FetchContent)
FetchContent_Declare(xterm_ctrl_seqs
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/../utils/xterm_ctrl_seqs/"
  )
FetchContent_MakeAvailable(xterm_ctrl_seqs)

# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
cmake_minimum_required(VERSION 3.16)

# TBD if using LocatePackageOrFetchContent, need to bypass find_package or
#   create find module for sdl2_smart_ptrs
#[[
include(LocatePackageOrFetchContent)
set(FC_OPTIONS
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/../utils/sdl2_smart_ptrs/"
  )
LocatePackageOrFetchContent(sdl2_smart_ptrs "" "${FC_OPTIONS}")
]]

include(FetchContent)
FetchContent_Declare(sdl2_smart_ptrs
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/../utils/sdl2_smart_ptrs/"
  )
FetchContent_MakeAvailable(sdl2_smart_ptrs)

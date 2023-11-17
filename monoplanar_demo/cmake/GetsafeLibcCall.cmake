# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14)

# TBD if using LocatePackageOrFetchContent, need to bypass find_package or
#   create find module for safeSdlCall
#[[
include(LocatePackageOrFetchContent)
set(FC_OPTIONS
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/../utils/safeLibcCall/"
  )
LocatePackageOrFetchContent(safeLibcCall "" "${FC_OPTIONS}")
]]

include(FetchContent)
FetchContent_Declare(safeLibcCall
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/../utils/safeLibcCall/"
  )
FetchContent_MakeAvailable(safeLibcCall)

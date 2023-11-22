# newest features used: TBD
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.16) #???

project(SafeLibcCall
  VERSION 2.0
  DESCRIPTION "Templated C++ wrapper for glibc function calls to throw \
    failures by return and/or errno values as exceptions"
  LANGUAGES C CXX
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(InSourceBuildGuard)

add_subdirectory(src)

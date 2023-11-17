# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14)

# cmake 3.16-supplied FindSDL*.cmake modules are for SDL1.x, _not_ SDL2!
include(LocatePackageOrFetchContent)

# !!! note option to set FETCH_ALL_DEPENDENCIES to OFF for testing

# TBD add unset of FP_OPTIONS to prevent accidental use of unknown strings

# SDL2 release installed by local package manager and used in testing, but
#   does not set targets SDL2::SDL2 and SDL2::SDL2main:
#   2.0.10 0e9560aea22818884921e5e5064953257bfe7fa7
# earliest SDL2 release to define targets SDL2::SDL2 and SDL2::SDL2main:
#   2.0.16 25f9ed87ff6947d9576fc9d79dee0784e638ac58
# SDL2 current release at time of writing:
#   2.28.3 8a5ba43d00252c6c8b33c9aa4f1048222955ab4d
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  # 2.0.16
  GIT_TAG        25f9ed87ff6947d9576fc9d79dee0784e638ac58
  )
LocatePackageOrFetchContent(SDL2 "${FP_OPTIONS}" "${FC_OPTIONS}")

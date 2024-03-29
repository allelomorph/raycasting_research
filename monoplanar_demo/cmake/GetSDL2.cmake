# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
cmake_minimum_required(VERSION 3.16)

# cmake 3.16-supplied FindSDL*.cmake modules are for SDL1.x, so to use SDL2 we
#   need to supply our own find modules

include(FetchIfNotFound)

# See https://packages.ubuntu.com/search?keywords=sdl2 for package versions
# In all cases currently only considering SDL2 v2, not v3

# Relevant SDL2 releases:
# - release matching Ubuntu 20.04 LTS package:
#   2.0.10 0e9560aea22818884921e5e5064953257bfe7fa7
# - earliest release to define targets SDL2::SDL2 and SDL2::SDL2main,
#   as well as define SDL_islower and SDL_isalpha for SDL_rtf 2.0.0
#   2.0.16 25f9ed87ff6947d9576fc9d79dee0784e638ac58
# - release matching Ubuntu 22.04 LTS package:
#   2.0.20 b424665e0899769b200231ba943353a5fee1b6b6
# - current release at last script update:
#   2.28.5 15ead9a40d09a1eb9972215cceac2bf29c9b77f6
set(FP_OPTIONS
  2.0.16
  )
# CMAKE_CACHE_ARGS not passed by FetchContent to ExternalProject_Add as documented in:
#   https://cmake.org/cmake/help/v3.16/module/FetchContent.html#command:fetchcontent_declare
#   https://cmake.org/cmake/help/v3.16/module/ExternalProject.html#command:externalproject_add
# So we set the cache variable directly instead of passing with `-D`, see:
#   https://discourse.cmake.org/t/fetchcontent-cache-variables/1538
set(SDL_STATIC OFF CACHE BOOL "Toggles building of SDL2 static library")
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        25f9ed87ff6947d9576fc9d79dee0784e638ac58 # 2.0.16
  )
fetch_if_not_found(SDL2 "${FP_OPTIONS}" "${FC_OPTIONS}")

# Relevant SDL2_image releases:
# - release matching Ubuntu 20.04 LTS and Ubuntu 22.04 LTS packages:
#   2.0.5 (not listed on github, commit hash unknown)
# - earliest listed release:
#   2.0.8 e9fc66a038304be0b892b83c16d6dcf5ee36f388
# - current release at last script update, requires SDL 2.0.9)
#   2.6.3 d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829
set(FP_OPTIONS
  2.0.5
  )
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG        d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829 # 2.6.3
  )
fetch_if_not_found(SDL2_image "${FP_OPTIONS}" "${FC_OPTIONS}")

# Relevant SDL2_ttf releases:
# - release matching Ubuntu 20.04 LTS package:
#   2.0.15 33cdd1881e31184b49a68b4890d1d256fc0c6dc1
# - release matching Ubuntu 22.04 LTS package:
#   2.0.18 3e702ed9bf400b0a72534f144b8bec46ee0416cb
# - current release at last script update: (requires SDL2 2.0.10)
#   2.20.2 89d1692fd8fe91a679bb943d377bfbd709b52c23
set(FP_OPTIONS
  2.0.15
  )
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG        89d1692fd8fe91a679bb943d377bfbd709b52c23 # 2.20.2
  )
fetch_if_not_found(SDL2_ttf "${FP_OPTIONS}" "${FC_OPTIONS}")

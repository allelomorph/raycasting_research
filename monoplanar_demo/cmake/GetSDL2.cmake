# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14)

# cmake 3.16-supplied FindSDL*.cmake modules are for SDL1.x, _not_ SDL2!

include(LocatePackageOrFetchContent)

# SDL2 current release at last script update:
#   2.28.3 8a5ba43d00252c6c8b33c9aa4f1048222955ab4d
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  # 2.0.16 (earliest release to define targets SDL2::SDL2 and SDL2::SDL2main)
  GIT_TAG        25f9ed87ff6947d9576fc9d79dee0784e638ac58
  )
LocatePackageOrFetchContent(SDL2 "" "${FC_OPTIONS}")

set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  # 2.6.3 (current release at last script update, requires SDL 2.0.9)
  GIT_TAG        d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829
  )
LocatePackageOrFetchContent(SDL2_image "" "${FC_OPTIONS}")

set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  # 2.20.2 (current release at last script update, requires SDL 2.0.10)
  GIT_TAG        89d1692fd8fe91a679bb943d377bfbd709b52c23
  )
LocatePackageOrFetchContent(SDL2_ttf "" "${FC_OPTIONS}")

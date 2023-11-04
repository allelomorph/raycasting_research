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

# SDL2_image release installed by local package manager and used in testing,
#   but has no CMakeLists.txt:
#   2.0.5 ab2a9c602623193d61827ccd395302d92d90fc38 (required SDL ver unknown)
# earliest SDL2_image release with CMakeLists.txt:
#   2.6.0 7b3347c0d90d1f1e9cc4e06e145432697ca4e68f (requires SDL 2.0.9)
# SDL2_image current release at time of writing:
#   2.6.3 d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829 (requires SDL 2.0.9)
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  # 2.6.3 (requires SDL 2.0.9)
  GIT_TAG        d3c6d5963dbe438bcae0e2b6f3d7cfea23d02829
  )
LocatePackageOrFetchContent(SDL2_image "${FP_OPTIONS}" "${FC_OPTIONS}")

# SDL2_ttf release installed by local package manager and used in testing,
#   but does not set target SDL2_ttf::SDL2_ttf:
#   2.0.15 f34e0a8e08efdb5e074731866cf8663f8c2c0baf (required SDL ver unknown)
# SDL2_ttf current release at time of writing:
#   2.20.2 89d1692fd8fe91a679bb943d377bfbd709b52c23 (requires SDL 2.0.10)
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  # 2.20.2 (requires SDL 2.0.10)
  GIT_TAG        89d1692fd8fe91a679bb943d377bfbd709b52c23
  )
LocatePackageOrFetchContent(SDL2_ttf "${FP_OPTIONS}" "${FC_OPTIONS}")

# newest features used: FetchContent v3.11, FetchContent_MakeAvailable v3.14
# targeting cmake v3.16
cmake_minimum_required(VERSION 3.14)

# cmake 3.16-supplied FindSDL*.cmake modules are for SDL1.x, _not_ SDL2!

include(LocatePackageOrFetchContent)

# SDL2 current v2 release at last script update:
#   2.28.5 15ead9a40d09a1eb9972215cceac2bf29c9b77f6
set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  # 2.0.16 (earliest release to define targets SDL2::SDL2 and SDL2::SDL2main)
  GIT_TAG        25f9ed87ff6947d9576fc9d79dee0784e638ac58
  )
LocatePackageOrFetchContent(SDL2 "" "${FC_OPTIONS}")

# SDL_image not needed, as it only allocates SDL_Surface

set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
  # 2.6.3 (current v2 release at last script update, requires SDL 2.0.9)
  GIT_TAG        6103316427a8479e5027e41ab9948bebfc1c3c19
  )
LocatePackageOrFetchContent(SDL2_mixer "" "${FC_OPTIONS}")

set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_net.git
  # 2.2.0 (current v2 release at last script update, requires SDL 2.0.4)
  GIT_TAG        669e75b84632e2c6cc5c65974ec9e28052cb7a4e
  )
LocatePackageOrFetchContent(SDL2_net "" "${FC_OPTIONS}")

set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  # 2.20.2 (current v2 release at last script update, requires SDL 2.0.10)
  GIT_TAG        89d1692fd8fe91a679bb943d377bfbd709b52c23
  )
LocatePackageOrFetchContent(SDL2_ttf "" "${FC_OPTIONS}")

set(FC_OPTIONS
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_rtf.git
  # 2.0.0 (current v2 release at last script update, requires SDL 2.0.0)
  GIT_TAG        db0e4676d6f9f6a271747ae21f997c3743cd53e1
  )
LocatePackageOrFetchContent(SDL2_rtf "" "${FC_OPTIONS}")

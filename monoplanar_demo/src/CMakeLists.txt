# TBD cmake version?

set(MPDEMO_OBJ
  App
  DdaRaycastEngine
  FpsCalc
  KbdInputMgr
  Layout
  LinuxKbdInputMgr
  SdlWindowMgr
  SdlKbdInputMgr
  TtyPixelBuffer
  TtyWindowMgr
  Vector2d
  main
  )

include(SetStrictCompileOptions)
foreach(target ${MPDEMO_OBJ})
add_library(${target} OBJECT
  "${target}.cc"
  )
set_target_properties(${target} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  )
set_strict_compile_options(${target})
target_include_directories(${target} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
endforeach()

#[[
target_precompile_headers(App
  PUBLIC
    "App.hh"
    "LinuxKbdInputMgr.hh"
    "SdlKbdInputMgr.hh"
  PRIVATE
    <SDL2/SDL.h>
    <SDL2/SDL_events.h>
    <SDL2/SDL_video.h>
    <csignal>
    <cstring>
    )
target_precompile_headers(DdaRaycastEngine
  PUBLIC
    "DdaRaycastEngine.hh"
  PRIVATE
    <cstdint>
    <cmath>
    )
target_precompile_headers(FpsCalc
  PUBLIC
    "FpsCalc.hh"
  PRIVATE
    <ctime>
    <chrono>
  )
target_precompile_headers(KbdInputMgr
  PUBLIC
    "KbdInputMgr.hh"
  )
target_precompile_headers(Layout
  PUBLIC
    "Layout.hh"
    "Vector2d.hh"
  PRIVATE
    <iostream>
    <algorithm>
    <fstream>
    <sstream>
    <string>
  )
target_precompile_headers(LinuxKbdInputMgr
  PUBLIC
    "LinuxKbdInputMgr.hh"
  PRIVATE
    <sys/types.h>
    <sys/stat.h>
    <fcntl.h>
    <sys/ioctl.h>
    <linux/input.h>
    <unistd.h>
    <utmp.h>
    <sys/select.h>
    <cstdlib>
    <cctype>
    <vector>
    <string>     # reuse from Layout?
    <algorithm>  # reuse from Layout?
    <fstream>    # reuse from Layout?
    <iostream>
  )
target_precompile_headers(SdlWindowMgr
  PUBLIC
    "SdlWindowMgr.hh"
  PRIVATE
    <SDL2/SDL_image.h>
    <SDL2/SDL_ttf.h>
    <string>      # reuse from LinuxKbdInputMgr?
    <algorithm>   # reuse from Layout?
    <iostream>    # reuse from LinuxKbdInputMgr?
    <cassert>
  )
target_precompile_headers(SdlKbdInputMgr
  PUBLIC
    "SdlKbdInputMgr.hh"
  PRIVATE
    <SDL2/SDL_keycode.h>
  )
target_precompile_headers(TtyPixelBuffer
  PUBLIC
    "TtyPixelBuffer.hh"
  PRIVATE
    <cassert>      # reuse from SdlWindowMgr?
    <cstdint>
    )
target_precompile_headers(TtyWindowMgr
  PUBLIC
    "TtyWindowMgr.hh"
  PRIVATE
    <SDL2/SDL_image.h>
    <termios.h>
    <sys/ioctl.h>  # reuse from LinuxKbdInputMgr?
    <unistd.h>     # reuse from LinuxKbdInputMgr?
    <linux/input-event-codes.h>
    <cassert>      # reuse from SdlWindowMgr?
    <cstdio>
    <cstring>      # reuse from App?
    <iostream>     # reuse from LinuxKbdInputMgr?
    <iomanip>
    <string>       # reuse from LinuxKbdInputMgr?
    <sstream>      # reuse from Layout?
    <algorithm>    # reuse from Layout?
  )
target_precompile_headers(Vector2d
  PUBLIC
    "Vector2d.hh"
  PRIVATE
    <cmath>      # reuse from DdaRaycastEngine?
  )
target_precompile_headers(main
  PUBLIC
    "App.hh"
    "WindowMgr.hh"
    "SdlWindowMgr.hh"
    "Settings.hh"
  PRIVATE
    <getopt.h>
    <cctype>       # reuse from LinuxKbdInputMgr?
    <iostream>     # reuse from LinuxKbdInputMgr?
    <string>       # reuse from LinuxKbdInputMgr?
  )
]]

target_link_libraries(App
  safeLibcCall
  safeSdlCall
  sdl2_smart_ptrs_shared  # WindowMgr.hh
  xterm_ctrl_seqs_shared
  SDL2::SDL2
  )
# target_link_libraries(DdaRaycastEngine cmath)
target_link_libraries(FpsCalc
  safeLibcCall
  )
target_link_libraries(KbdInputMgr
  SDL2::SDL2  # KbdInputMgr.hh
  )
target_link_libraries(LinuxKbdInputMgr
  safeLibcCall
  )
target_link_libraries(SdlWindowMgr
  safeSdlCall
  sdl2_smart_ptrs_shared  # WindowMgr.hh
  SDL2_image::SDL2_image
  SDL2_ttf::SDL2_ttf
  )
target_link_libraries(SdlKbdInputMgr
  SDL2::SDL2
  )
target_link_libraries(TtyPixelBuffer
  xterm_ctrl_seqs_shared  # TtyPixelBuffer.hh
  )
target_link_libraries(TtyWindowMgr
  safeSdlCall
  safeLibcCall
  sdl2_smart_ptrs_shared  # WindowMgr.hh
  xterm_ctrl_seqs_shared
  SDL2::SDL2   # TtyWindowMgr.hh
  SDL2_image::SDL2_image
  )
target_link_libraries(main
  sdl2_smart_ptrs_shared  # WindowMgr.hh
  xterm_ctrl_seqs_shared
  )

set(MPDEMO_HEADER_ONLY_SRC
  KeyState.hh
  Settings.hh
  WindowMgr.hh
  )

add_executable(monoplanar_demo)
target_link_libraries(monoplanar_demo
  "${MPDEMO_OBJ}"
  #[[
  SDL2::SDL2
  SDL2_image::SDL2_image
  SDL2_ttf::SDL2_ttf
  safeLibcCall
  safeSdlCall
  sdl2_smart_ptrs_shared
  xterm_ctrl_seqs_shared
  ]]
  )

# note: Debug/Release modes can be used with -DCMAKE_BUILD_TYPE=(Debug|Release)
#    list(APPEND CMAKE_CXX_FLAGS_DEBUG
#      -Og --coverage

####
# Script Debug
####

message("testing targets in src")
include(PrintTargetProperties)
foreach(target
    ${MPDEMO_OBJ}
    SDL2
    SDL2main
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image
    SDL2_image::SDL2_image
    SDL2_ttf
    SDL2_ttf::SDL2_ttf
    errnoname_obj
    errnoname_static
    errnoname_shared
    safeLibcCall
    safeSdlCall
    sdl2_smart_ptrs_obj
    sdl2_smart_ptrs_static
    sdl2_smart_ptrs_shared
    xterm_ctrl_seqs_obj
    xterm_ctrl_seqs_static
    xterm_ctrl_seqs_shared
    )
  print_target_properties(${target})
endforeach()

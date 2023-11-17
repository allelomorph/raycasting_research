# set least error-tolerant options for a target, based on complier in use
macro(set_strict_compile_options target)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${target} PRIVATE
      /W4 /bigobj
      )
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic
      )
  endif()

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Werror
      )
  endif()
endmacro()

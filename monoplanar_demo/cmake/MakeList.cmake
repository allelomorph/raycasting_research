# https://stackoverflow.com/a/53278726
function(MakeList LIST)
  unset(${LIST})
  foreach(ARG ${ARGN})
    list(APPEND ${LIST} ${ARG})
  endforeach()
  set(${LIST} "${${LIST}}" PARENT_SCOPE)
endfunction()

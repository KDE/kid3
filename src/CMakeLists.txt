if (WITH_GCC_PCH)
  if (WITH_KDE)
    set(_includeRegex "#include <[Qkcp]")
  else (WITH_KDE)
    set(_includeRegex "#include <[Qcp]")
  endif (WITH_KDE)
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(_additionalFlags "-g -DQT_DEBUG -D_REENTRANT")
  else (CMAKE_BUILD_TYPE MATCHES Debug)
    set(_additionalFlags "-O3 -DQT_NO_DEBUG -D_REENTRANT -DNDEBUG")
  endif (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Winvalid-pch -include ${CMAKE_BINARY_DIR}/allsys.h")

  macro (ADD_KID3_PCH_RULE _srcList)
    ADD_PCH_HEADERFILE_RULE(${CMAKE_BINARY_DIR}/allsys.h
                            ${CMAKE_CURRENT_SOURCE_DIR} ${_includeRegex})
    ADD_PCH_RULE(${CMAKE_BINARY_DIR}/allsys.h ${_srcList} ${_additionalFlags})
  endmacro (ADD_KID3_PCH_RULE _srcList)
endif (WITH_GCC_PCH)

add_subdirectory(core)
add_subdirectory(gui)

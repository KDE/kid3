if (WITH_GCC_PCH)
  if (BUILD_KDE_APP)
    set(_includeRegex "#include <[Qkcp]")
  else (BUILD_KDE_APP)
    set(_includeRegex "#include <[Qcp]")
  endif (BUILD_KDE_APP)
  set(_additionalFlags "-fno-exceptions -D_REENTRANT -DQT_NO_EXCEPTIONS -DQT_NO_STL -fvisibility=hidden -fvisibility-inlines-hidden -DQT_ASCII_CAST_WARNINGS -DQT_NO_CAST_TO_ASCII")
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(_additionalFlags "${_additionalFlags} -g -DQT_DEBUG")
  else (CMAKE_BUILD_TYPE MATCHES Debug)
    set(_additionalFlags "${_additionalFlags} -O3 -DQT_NO_DEBUG -DNDEBUG")
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
add_subdirectory(app)

if (BUILD_TEST_APP)
  add_subdirectory(test)
endif (BUILD_TEST_APP)

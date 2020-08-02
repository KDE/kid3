# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
if(WIN32 OR APPLE OR CMAKE_CXX_COMPILER MATCHES "/osxcross/")
  find_library(TAGLIB_LIBRARY NAMES tag)
  find_path(TAGLIB_INCLUDE_DIR taglib/taglib.h)
  if(TAGLIB_LIBRARY AND TAGLIB_INCLUDE_DIR)
    message(STATUS "TagLib found: ${TAGLIB_LIBRARY}")
    set(HAVE_TAGLIB 1)
    set(TAGLIB_INCLUDES ${TAGLIB_INCLUDE_DIR}/taglib ${TAGLIB_INCLUDE_DIR})
    set(TAGLIB_DEFINITIONS -DTAGLIB_STATIC)
    set(TAGLIB_INTERFACE_LIBRARIES ${ZLIB_LIBRARIES})
    set(TAGLIB_LIBRARIES ${TAGLIB_LIBRARY} ${TAGLIB_INTERFACE_LIBRARIES})
  endif()
else()
  if(WITH_TAGLIB)
    find_program(TAGLIBCONFIG_EXECUTABLE NAMES taglib-config PATHS /usr/bin /usr/local/bin ${BIN_INSTALL_DIR})
    if(TAGLIBCONFIG_EXECUTABLE)
      exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_LDFLAGS)
      exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_CFLAGS)
      exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_VERSION)
      set(TAGLIB_DEFINITIONS)
    endif()
  elseif(TAGLIB_LIBRARIES)
    # WITH_TAGLIB is set OFF, but the variables HAVE_TAGLIB, TAGLIB_LIBRARIES,
    # TAGLIB_CFLAGS, TAGLIB_VERSION are explicitly set.
    set(TAGLIB_LDFLAGS ${TAGLIB_LIBRARIES})
    string(REGEX MATCH "-D[^ ]+" TAGLIB_DEFINITIONS ${TAGLIB_CFLAGS})
  endif()
  if(TAGLIB_LDFLAGS AND TAGLIB_VERSION)
    if(NOT ${TAGLIB_VERSION} VERSION_LESS 1.9.1)
      # Extract library name and path from TAGLIB_LDFLAGS, which has
      # the format "-L/usr/lib -ltag"
      string(REGEX MATCH "-L *([^ ]+) +-l *([^ ]+)" _match ${TAGLIB_LDFLAGS})
      if(_match)
        find_library(TAGLIB_LIBRARY NAMES ${CMAKE_MATCH_2}
                                    HINTS ${CMAKE_MATCH_1}
                                    NO_CMAKE_FIND_ROOT_PATH)
      else()
        # Maybe TAGLIB_LDFLAGS has the format "-ltag" as on Red Hat
        string(REGEX MATCH "-l *([^ ]+)" _match ${TAGLIB_LDFLAGS})
        if(_match)
          find_library(TAGLIB_LIBRARY NAMES ${CMAKE_MATCH_1}
                                      NO_CMAKE_FIND_ROOT_PATH)
       endif()
      endif()
      # Extract include path from TAGLIB_CFLAGS, which has the format
      # "-I/usr/include/taglib"
      string(REGEX MATCH "-I *([^ ]+)" _match ${TAGLIB_CFLAGS})
      if(_match)
        set(TAGLIB_INCLUDE_DIR ${CMAKE_MATCH_1})
      endif()
      if(TAGLIB_LIBRARY AND TAGLIB_INCLUDE_DIR)
        message(STATUS "TagLib found: ${TAGLIB_LIBRARY}")
        set(HAVE_TAGLIB 1)
        set(TAGLIB_INCLUDES ${TAGLIB_INCLUDE_DIR})
        set(TAGLIB_INTERFACE_LIBRARIES ${ZLIB_LIBRARIES})
        if(NOT MSVC)
          set(TAGLIB_INTERFACE_LIBRARIES ${TAGLIB_INTERFACE_LIBRARIES} -lstdc++)
        endif()
        set(TAGLIB_LIBRARIES ${TAGLIB_LIBRARY} ${TAGLIB_INTERFACE_LIBRARIES})
      endif()
    endif()
  endif()
endif()
if(NOT HAVE_TAGLIB)
  message(FATAL_ERROR "Could not find Taglib")
endif()

set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${TAGLIB_LIBRARIES})
set(_CMAKE_REQUIRED_DEFINITIONS_TMP ${CMAKE_REQUIRED_DEFINITIONS})
set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${TAGLIB_DEFINITIONS})
set(_CMAKE_REQUIRED_INCLUDES_TMP ${CMAKE_REQUIRED_INCLUDES})
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${TAGLIB_INCLUDES})
CHECK_CXX_SOURCE_COMPILES("#include <mpegfile.h>\nint main() {\n  TagLib::MPEG::File file(\"somefile.mp3\");\n  file.save(3, false, 3);\n  return 0;\n}\n" HAVE_TAGLIB_ID3V23_SUPPORT)
CHECK_CXX_SOURCE_COMPILES("#include <mpegfile.h>\n#include <xmfile.h>\nint main() {\n  TagLib::MPEG::File file(\"somefile.mp3\");\n  return dynamic_cast<TagLib::XM::Properties*>(file.audioProperties()) != 0;\n}\n" HAVE_TAGLIB_XM_SUPPORT)
set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
set(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_TMP})
set(CMAKE_REQUIRED_INCLUDES ${_CMAKE_REQUIRED_INCLUDES_TMP})

if(NOT TARGET TagLib::TagLib)
  add_library(TagLib::TagLib UNKNOWN IMPORTED)
  string(REPLACE "-D" "" TAGLIB_COMPILE_DEFINITIONS "${TAGLIB_DEFINITIONS}")
  set_target_properties(TagLib::TagLib PROPERTIES
    IMPORTED_LOCATION "${TAGLIB_LIBRARY}"
    INTERFACE_COMPILE_DEFINITIONS "${TAGLIB_COMPILE_DEFINITIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${TAGLIB_INCLUDES}"
    INTERFACE_LINK_LIBRARIES "${TAGLIB_INTERFACE_LIBRARIES}"
  )
endif()

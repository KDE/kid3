set(TAGLIB_LIBRARIES)
set(TAGLIB_CFLAGS)
if(WITH_TAGLIB)
  if (WIN32)
    find_library(TAGLIB_LIBRARIES NAMES tag)
    find_path(TAGLIB_INCLUDE_DIR taglib/taglib.h)
    if (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIR)
      set(TAGLIB_CFLAGS -I${TAGLIB_INCLUDE_DIR}/taglib -I${TAGLIB_INCLUDE_DIR} -DTAGLIB_STATIC)
      set(HAVE_TAGLIB 1)
      message(STATUS "TagLib found: ${TAGLIB_LIBRARIES}")
      set(TAGLIB_LIBRARIES ${TAGLIB_LIBRARIES} ${ZLIB_LIBRARIES})
    endif (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIR)
  else (WIN32)
    find_program(TAGLIBCONFIG_EXECUTABLE NAMES taglib-config PATHS /usr/bin /usr/local/bin ${BIN_INSTALL_DIR})
    if(TAGLIBCONFIG_EXECUTABLE)
      exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_LIBRARIES)
      exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_CFLAGS)
      exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_VERSION)
      if(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS AND TAGLIB_VERSION)
        if(NOT ${TAGLIB_VERSION} VERSION_LESS 1.4)
          set(HAVE_TAGLIB 1)
          message(STATUS "TagLib found: ${TAGLIB_LIBRARIES}")
          set(TAGLIB_LIBRARIES ${TAGLIB_LIBRARIES} ${ZLIB_LIBRARIES} -lstdc++)
        endif(NOT ${TAGLIB_VERSION} VERSION_LESS 1.4)
      endif(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS AND TAGLIB_VERSION)
    endif(TAGLIBCONFIG_EXECUTABLE)
  endif (WIN32)
  if (NOT HAVE_TAGLIB)
    message(FATAL_ERROR "Could not find Taglib")
  endif (NOT HAVE_TAGLIB)
endif(WITH_TAGLIB)
if (TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
  set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${TAGLIB_LIBRARIES})
  set(_CMAKE_REQUIRED_DEFINITIONS_TMP ${CMAKE_REQUIRED_DEFINITIONS})
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${TAGLIB_CFLAGS})
  CHECK_CXX_SOURCE_COMPILES("#include <mpegfile.h>\nint main() {\n  TagLib::MPEG::File file(\"somefile.mp3\");\n  file.save(3, false, 3);\n  return 0;\n}\n" HAVE_TAGLIB_ID3V23_SUPPORT)
  CHECK_CXX_SOURCE_COMPILES("#include <mpegfile.h>\n#include <xmfile.h>\nint main() {\n  TagLib::MPEG::File file(\"somefile.mp3\");\n  return dynamic_cast<TagLib::XM::Properties*>(file.audioProperties()) != 0;\n}\n" HAVE_TAGLIB_XM_SUPPORT)
  CHECK_CXX_SOURCE_COMPILES("#include <podcastframe.h>\nint main() {\n  TagLib::ID3v2::PodcastFrame f;;\n  return 0;\n}\n" HAVE_TAGLIB_PODCAST_SUPPORT)
  set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
  set(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_TMP})


  configure_file(taglibconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/taglibconfig.h)

  add_definitions(${TAGLIB_CFLAGS})
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ../../core/tags taglibext)

  set(plugin_SRCS
    taglibmetadataplugin.cpp
    taglibfile.cpp
    taglibext/aac/aacfiletyperesolver.cpp
    taglibext/mp2/mp2filetyperesolver.cpp
  )

  if (NOT ${TAGLIB_VERSION} VERSION_GREATER 1.9.1)
    set(plugin_SRCS ${plugin_SRCS}
      taglibext/synchronizedlyricsframe.cpp
      taglibext/eventtimingcodesframe.cpp
    )
  endif (NOT ${TAGLIB_VERSION} VERSION_GREATER 1.9.1)

  if (NOT ${TAGLIB_VERSION} VERSION_LESS 1.9.1)
    set(plugin_SRCS ${plugin_SRCS}
      taglibext/dsf/dsffiletyperesolver.cpp
      taglibext/dsf/dsffile.cpp
      taglibext/dsf/dsfproperties.cpp
      taglibext/dsf/dsfheader.cpp
    )
  endif (NOT ${TAGLIB_VERSION} VERSION_LESS 1.9.1)

  if (TAGLIB_VERSION STREQUAL "1.4")
    set(plugin_SRCS ${plugin_SRCS}
      taglibext/urllinkframe.cpp
      taglibext/unsynchronizedlyricsframe.cpp
      taglibext/generalencapsulatedobjectframe.cpp
      taglibext/speex/speexfile.cpp
      taglibext/speex/speexproperties.cpp
      taglibext/speex/taglib_speexfiletyperesolver.cpp
      taglibext/trueaudio/taglib_trueaudiofiletyperesolver.cpp
      taglibext/trueaudio/ttafile.cpp
      taglibext/trueaudio/ttaproperties.cpp
      taglibext/wavpack/taglib_wavpackfiletyperesolver.cpp
      taglibext/wavpack/wvfile.cpp
      taglibext/wavpack/wvproperties.cpp
    )
  endif (TAGLIB_VERSION STREQUAL "1.4")

  set(plugin_MOC_HDRS
    taglibmetadataplugin.h
  )

  set(plugin_NAME TaglibMetadata)

  if (WITH_GCC_PCH)
    add_definitions(${GCC_PCH_COMPILE_FLAGS})
  endif (WITH_GCC_PCH)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  qt4_wrap_cpp(plugin_GEN_MOC_SRCS ${plugin_MOC_HDRS})

  add_library(${plugin_TARGET} ${plugin_SRCS} ${plugin_GEN_MOC_SRCS})
  target_link_libraries(${plugin_TARGET} kid3-core ${BASIC_LIBRARIES} ${TAGLIB_LIBRARIES})

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif (TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)

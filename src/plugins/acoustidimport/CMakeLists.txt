if(WITH_CHROMAPRINT)

  macro(find_gstreamer)
    find_package(PkgConfig)
    set(WITH_GSTREAMER_VERSION "" CACHE STRING
      "GStreamer version to use or list of versions to check")
    if (WITH_GSTREAMER_VERSION)
      set(_checkedGstVersions ${WITH_GSTREAMER_VERSION})
    else (WITH_GSTREAMER_VERSION)
      # Prefer version 0.10 as long as Phonon and QtMultimedia
      # are using it.
      set(_checkedGstVersions "0.10;1.0")
    endif (WITH_GSTREAMER_VERSION)
    foreach (_gstVersion ${_checkedGstVersions})
      pkg_check_modules(GSTREAMER gstreamer-${_gstVersion})
      if (GSTREAMER_FOUND)
        if (NOT WITH_GSTREAMER_VERSION)
          message(STATUS "GStreamer ${_gstVersion} selected, "
            "use WITH_GSTREAMER_VERSION for another version.")
        endif (NOT WITH_GSTREAMER_VERSION)
        if (_gstVersion STREQUAL "1.0" AND WITH_PHONON)
          message(WARNING "If Phonon uses a different GStreamer version "
            "it will probably hang when playing.")
        endif ()
        break()
      endif (GSTREAMER_FOUND)
    endforeach (_gstVersion ${WITH_GSTREAMER_VERSION})
  endmacro(find_gstreamer)

  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
  find_package(Chromaprint REQUIRED)
  if(CHROMAPRINT_INCLUDE_DIR AND CHROMAPRINT_LIBRARIES)
    set(HAVE_CHROMAPRINT 1)
  endif(CHROMAPRINT_INCLUDE_DIR AND CHROMAPRINT_LIBRARIES)
  set(CHROMAPRINT_INCLUDES ${CHROMAPRINT_INCLUDE_DIR})
  set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBRARIES})
  if(WITH_FFMPEG)
    find_package(FFmpeg REQUIRED)
  elseif(WITH_GSTREAMER)
    find_gstreamer()
    if (NOT GSTREAMER_FOUND)
      message(FATAL_ERROR "Could not find GStreamer")
    endif (NOT GSTREAMER_FOUND)
  elseif(HAVE_QT5 AND WITH_QAUDIODECODER)
    message(STATUS "QAudioDecoder selected for decoding.")
  else(WITH_FFMPEG)
    # Neither FFmpeg nor GStreamer is forced, select FFmpeg if Chromaprint
    # depends on it or if on Windows or Mac OS X.
    include(GetPrerequisites)
    get_prerequisites(${CHROMAPRINT_LIBRARIES} _chromaprintDeps 0 0 "" "")
    if(_chromaprintDeps MATCHES "libavcodec" OR APPLE OR WIN32)
      message(STATUS "FFmpeg selected for decoding, use WITH_GSTREAMER=ON to use GStreamer instead.")
      find_package(FFmpeg REQUIRED)
    else()
      message(STATUS "GStreamer selected for decoding, use WITH_FFMPEG=ON to use FFmpeg instead.")
      find_gstreamer()
    endif()
  endif(WITH_FFMPEG)
  if(GSTREAMER_FOUND)
    set(HAVE_GSTREAMER 1)
    set(CHROMAPRINT_CFLAGS ${GSTREAMER_CFLAGS_OTHER})
    set(CHROMAPRINT_INCLUDES ${CHROMAPRINT_INCLUDES} ${GSTREAMER_INCLUDE_DIRS})
    set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${GSTREAMER_LDFLAGS})
  elseif(FFMPEG_LIBAVFORMAT_FOUND AND FFMPEG_LIBAVCODEC_FOUND AND FFMPEG_LIBAVUTIL_FOUND)
    if (FFMPEG_LIBAVRESAMPLE_FOUND)
      set(HAVE_AVRESAMPLE 1)
      set(CHROMAPRINT_INCLUDES ${CHROMAPRINT_INCLUDES} ${FFMPEG_LIBAVRESAMPLE_INCLUDE_DIRS})
      set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${FFMPEG_LIBAVRESAMPLE_LIBRARIES})
    else (FFMPEG_LIBAVRESAMPLE_FOUND)
      include(CheckFunctionExists)
      set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
      set(CMAKE_REQUIRED_LIBRARIES
        ${FFMPEG_LIBAVFORMAT_LIBRARIES}
        ${FFMPEG_LIBAVCODEC_LIBRARIES}
        ${FFMPEG_LIBAVUTIL_LIBRARIES})
      check_function_exists(av_audio_convert HAVE_AV_AUDIO_CONVERT)
      set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
    endif (FFMPEG_LIBAVRESAMPLE_FOUND)
    set(HAVE_FFMPEG 1)

    set(CHROMAPRINT_INCLUDES ${CHROMAPRINT_INCLUDES} ${FFMPEG_LIBAVCODEC_INCLUDE_DIRS} ${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS} ${FFMPEG_LIBAVUTIL_INCLUDE_DIRS})
    set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${FFMPEG_LIBAVFORMAT_LIBRARIES} ${FFMPEG_LIBAVCODEC_LIBRARIES} ${FFMPEG_LIBAVUTIL_LIBRARIES})
    if(WIN32 OR APPLE)
      set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${ZLIB_LIBRARIES})
    endif(WIN32 OR APPLE)
  elseif(WITH_CHROMAPRINT_FFMPEG)
    find_package(FFmpeg REQUIRED)
    set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${FFMPEG_LIBAVCODEC_LIBRARIES} ${FFMPEG_LIBAVUTIL_LIBRARIES})
  endif(GSTREAMER_FOUND)
  if (APPLE)
    find_library(ACCELERATE_LIBRARIES Accelerate)
    set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${ACCELERATE_LIBRARIES})
  endif (APPLE)

  configure_file(acoustidconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/acoustidconfig.h)

  add_definitions(${CHROMAPRINT_CFLAGS})
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CHROMAPRINT_INCLUDES})

  if (WIN32 AND HAVE_CHROMAPRINT)
    add_definitions(-DCHROMAPRINT_NODLL)
  endif (WIN32 AND HAVE_CHROMAPRINT)

  set(plugin_SRCS
    abstractfingerprintdecoder.cpp
    fingerprintcalculator.cpp
    musicbrainzclient.cpp
    acoustidimportplugin.cpp
  )

  set(plugin_MOC_HDRS
    abstractfingerprintdecoder.h
    fingerprintcalculator.h
    musicbrainzclient.h
    acoustidimportplugin.h
  )

  if (HAVE_GSTREAMER)
    set(plugin_SRCS ${plugin_SRCS} gstfingerprintdecoder.cpp)
  elseif (HAVE_FFMPEG)
    set(plugin_SRCS ${plugin_SRCS} ffmpegfingerprintdecoder.cpp)
  elseif (HAVE_QT5)
    set(plugin_SRCS ${plugin_SRCS} qtfingerprintdecoder.cpp)
    set(plugin_MOC_HDRS ${plugin_MOC_HDRS} qtfingerprintdecoder.h)
  endif ()

  set(plugin_NAME AcoustidImport)

  if (WITH_GCC_PCH)
    add_definitions(${GCC_PCH_COMPILE_FLAGS})
  endif (WITH_GCC_PCH)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  qt4_wrap_cpp(plugin_GEN_MOC_SRCS ${plugin_MOC_HDRS})

  add_library(${plugin_TARGET} ${plugin_SRCS} ${plugin_GEN_MOC_SRCS})
  target_link_libraries(${plugin_TARGET} kid3-core ${BASIC_LIBRARIES} ${CHROMAPRINT_LIBS})
  if (APPLE AND CMAKE_EXE_LINKER_FLAGS MATCHES "-arch i386")
    # To suppress linker error
    # ld: illegal text-relocation to non_lazy_ptr in ../libavformat.a
    # (allformats.o) from _av_register_all in ../libavformat.a(allformats.o)
    # for architecture i386
    SET_TARGET_PROPERTIES(${plugin_TARGET} PROPERTIES LINK_FLAGS -Wl,-read_only_relocs,suppress)
  endif (APPLE AND CMAKE_EXE_LINKER_FLAGS MATCHES "-arch i386")

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif(WITH_CHROMAPRINT)

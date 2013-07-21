if(WITH_CHROMAPRINT)
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
    find_package(PkgConfig)
    pkg_check_modules(GSTREAMER gstreamer-1.0)
    if (NOT GSTREAMER_FOUND)
      pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10)
    endif (NOT GSTREAMER_FOUND)
  elseif(HAVE_QT5 AND WITH_QAUDIODECODER)
    message(STATUS "QAudioDecoder selected for decoding.")
  else(WITH_FFMPEG)
    # Neither FFmpeg nor GStreamer is forced, select FFmpeg if Chromaprint
    # depends on it or if on Windows or Mac OS X.
    include(GetPrerequisites)
    get_prerequisites(${CHROMAPRINT_LIBRARIES} _chromaprintDeps 0 0 "" "")
    string(FIND "${_chromaprintDeps}" "libavcodec" _avPos)
    if(${_avPos} GREATER -1 OR APPLE OR WIN32)
      message(STATUS "FFmpeg selected for decoding, use WITH_GSTREAMER=ON to use GStreamer instead.")
      find_package(FFmpeg REQUIRED)
    else()
      message(STATUS "GStreamer selected for decoding, use WITH_FFMPEG=ON to use FFmpeg instead.")
      find_package(PkgConfig)
      pkg_check_modules(GSTREAMER gstreamer-1.0)
      if (NOT GSTREAMER_FOUND)
        pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10)
      endif (NOT GSTREAMER_FOUND)
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
    ADD_KID3_PCH_RULE(plugin_SRCS)
  endif (WITH_GCC_PCH)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  qt4_wrap_cpp(plugin_GEN_MOC_SRCS ${plugin_MOC_HDRS})

  add_library(${plugin_TARGET} ${plugin_SRCS} ${plugin_GEN_MOC_SRCS})
  target_link_libraries(${plugin_TARGET} kid3-core ${BASIC_LIBRARIES} ${CHROMAPRINT_LIBS})

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif(WITH_CHROMAPRINT)

if(WITH_CHROMAPRINT)

  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
  find_package(Chromaprint REQUIRED)
  if(CHROMAPRINT_INCLUDE_DIR AND CHROMAPRINT_LIBRARIES)
    set(HAVE_CHROMAPRINT 1)
  endif()
  if(WITH_FFMPEG)
    find_package(FFmpeg COMPONENTS avformat avcodec avutil REQUIRED)
  elseif(WITH_GSTREAMER)
    find_package(GStreamer REQUIRED)
    if(NOT GSTREAMER_FOUND)
      message(FATAL_ERROR "Could not find GStreamer")
    endif()
  elseif(WITH_QAUDIODECODER)
    message(STATUS "QAudioDecoder selected for decoding.")
  else()
    # Neither FFmpeg nor GStreamer is forced, select FFmpeg if Chromaprint
    # depends on it or if on Windows or Mac OS X.
    include(GetPrerequisites)
    get_prerequisites(${CHROMAPRINT_LIBRARIES} _chromaprintDeps 0 0 "" "")
    if(_chromaprintDeps MATCHES "libavcodec" OR APPLE OR WIN32)
      message(STATUS "FFmpeg selected for decoding, use WITH_GSTREAMER=ON to use GStreamer instead.")
      find_package(FFmpeg COMPONENTS avformat avcodec avutil REQUIRED)
    else()
      message(STATUS "GStreamer selected for decoding, use WITH_FFMPEG=ON to use FFmpeg instead.")
      find_package(GStreamer REQUIRED)
    endif()
  endif()
  if(GSTREAMER_FOUND)
    set(HAVE_GSTREAMER 1)
  elseif(FFMPEG_FOUND)
    set(HAVE_FFMPEG 1)
  endif()

  set(plugin_NAME AcoustidImport)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  add_library(${plugin_TARGET}
    abstractfingerprintdecoder.cpp
    fingerprintcalculator.cpp
    musicbrainzclient.cpp
    acoustidimportplugin.cpp
  )

  if(HAVE_GSTREAMER)
    target_sources(${plugin_TARGET} PRIVATE gstfingerprintdecoder.cpp)
  elseif(HAVE_FFMPEG)
    target_sources(${plugin_TARGET} PRIVATE ffmpegfingerprintdecoder.cpp)
  else()
    target_sources(${plugin_TARGET} PRIVATE qtfingerprintdecoder.cpp)
  endif()

  qt5_wrap_cpp(plugin_GEN_MOC_SRCS
    abstractfingerprintdecoder.h
    fingerprintcalculator.h
    musicbrainzclient.h
    acoustidimportplugin.h
    TARGET ${plugin_TARGET}
  )
  target_sources(${plugin_TARGET} PRIVATE ${plugin_GEN_MOC_SRCS})

  if(NOT HAVE_GSTREAMER AND NOT HAVE_FFMPEG)
    qt5_wrap_cpp(qtfp_GEN_MOC_SRCS qtfingerprintdecoder.h TARGET ${plugin_TARGET})
    target_sources(${plugin_TARGET} PRIVATE ${qtfp_GEN_MOC_SRCS})
  endif()

  target_include_directories(${plugin_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(${plugin_TARGET} PRIVATE kid3-core Chromaprint::Chromaprint Kid3Plugin)

  if(HAVE_GSTREAMER)
    target_link_libraries(${plugin_TARGET} PRIVATE GStreamer::GStreamer)
  elseif(HAVE_FFMPEG)
    if(FFmpeg_avformat_FOUND AND FFmpeg_avcodec_FOUND AND FFmpeg_avutil_FOUND)
      if(FFmpeg_swresample_FOUND)
        set(HAVE_SWRESAMPLE 1)
        target_link_libraries(${plugin_TARGET} PRIVATE FFmpeg::swresample)
      elseif(FFmpeg_avresample_FOUND)
        set(HAVE_AVRESAMPLE 1)
        target_link_libraries(${plugin_TARGET} PRIVATE FFmpeg::avresample)
      else()
        include(CheckFunctionExists)
        set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
        set(CMAKE_REQUIRED_LIBRARIES FFmpeg::avformat FFmpeg::avcodec FFmpeg::avutil)
        check_function_exists(av_audio_convert HAVE_AV_AUDIO_CONVERT)
        set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
      endif()
    endif()

    target_link_libraries(${plugin_TARGET} PRIVATE FFmpeg::avformat FFmpeg::avcodec FFmpeg::avutil)
    if(WIN32 OR APPLE)
      target_link_libraries(${plugin_TARGET} PRIVATE ${ZLIB_LIBRARIES})
    endif()
    if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
      # With MinGW64, FFmpeg 4 is used, which requires bcrypt
      target_link_libraries(${plugin_TARGET} PRIVATE "-lbcrypt")
    endif()
  else()
    target_link_libraries(${plugin_TARGET} PRIVATE Qt5::Multimedia)
  endif()

  if(APPLE)
    find_library(ACCELERATE_LIBRARIES Accelerate)
    target_link_libraries(${plugin_TARGET} PRIVATE ${ACCELERATE_LIBRARIES})
  endif()

  configure_file(acoustidconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/acoustidconfig.h)

  if(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
    # To suppress linker error
    # ld: illegal text-relocation to non_lazy_ptr in ../libavformat.a
    # (allformats.o) from _av_register_all in ../libavformat.a(allformats.o)
    # for architecture i386
    # ld: illegal text-relocation to _cpy8 in /usr/local/lib/libswresample.a
    # (audioconvert.o) from _swri_audio_convert_alloc in
    # /usr/local/lib/libswresample.a(audioconvert.o) for architecture i386
    set_target_properties(${plugin_TARGET} PROPERTIES LINK_FLAGS -Wl,-read_only_relocs,suppress)
  endif()

  if(LINUX_SELF_CONTAINED)
    # To avoid linker error
    # relocation R_X86_64_PC32 against symbol `ff_pw_9' can not be used when
    # making a shared object; recompile with -fPIC
    # see https://www.ffmpeg.org/platform.html#Advanced-linking-configuration
    set_target_properties(${plugin_TARGET} PROPERTIES LINK_FLAGS -Wl,-Bsymbolic)
  endif()

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif()

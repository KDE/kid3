set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

if(WITH_VORBIS)
  find_package(Vorbis REQUIRED)
endif()
if(WITH_FLAC)
  find_package(FLAC REQUIRED)
endif()

if(WITH_VORBIS OR WITH_FLAC)
  configure_file(oggflacconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/oggflacconfig.h)

  set(plugin_NAME OggFlacMetadata)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  add_library(${plugin_TARGET} oggflacmetadataplugin.cpp)
  if(HAVE_VORBIS)
    target_sources(${plugin_TARGET} PRIVATE oggfile.cpp vcedit.c)
  endif()
  if(HAVE_FLAC)
    target_sources(${plugin_TARGET} PRIVATE flacfile.cpp)
  endif()
  qt_wrap_cpp(plugin_GEN_MOC_SRCS
    oggflacmetadataplugin.h
    TARGET ${plugin_TARGET}
  )
  target_sources(${plugin_TARGET} PRIVATE ${plugin_GEN_MOC_SRCS})

  target_include_directories(${plugin_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

  target_link_libraries(${plugin_TARGET} kid3-core Kid3Plugin)

  if(HAVE_VORBIS)
    target_link_libraries(${plugin_TARGET} VorbisFile::VorbisFile)
  endif()
  if(HAVE_FLAC)
    target_link_libraries(${plugin_TARGET} FLACPP::FLACPP)
  endif()

  target_compile_definitions(${plugin_TARGET} PRIVATE HAVE_CONFIG_H)

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif()

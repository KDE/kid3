if(WITH_VORBIS)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

  find_package(Vorbis REQUIRED)
  find_package(FLAC)

  configure_file(oggflacconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/oggflacconfig.h)

  set(plugin_NAME OggFlacMetadata)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  add_library(${plugin_TARGET}
    oggflacmetadataplugin.cpp
    oggfile.cpp
    vcedit.c
  )
  if(HAVE_FLAC)
    target_sources(${plugin_TARGET} PRIVATE
      flacfile.cpp
    )
  endif()
  qt5_wrap_cpp(plugin_GEN_MOC_SRCS
    oggflacmetadataplugin.h
    TARGET ${plugin_TARGET}
  )
  target_sources(${plugin_TARGET} PRIVATE ${plugin_GEN_MOC_SRCS})

  target_include_directories(${plugin_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

  target_link_libraries(${plugin_TARGET} kid3-core VorbisFile::VorbisFile FLACPP::FLACPP Kid3Plugin)

  target_compile_definitions(${plugin_TARGET} PRIVATE HAVE_CONFIG_H)

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif()

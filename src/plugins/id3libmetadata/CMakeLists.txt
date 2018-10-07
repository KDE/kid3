if(WITH_ID3LIB)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

  find_package(Id3lib REQUIRED)
  
  configure_file(id3libconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/id3libconfig.h)

  set(plugin_NAME Id3libMetadata)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  qt5_wrap_cpp(plugin_GEN_MOC_SRCS
    id3libmetadataplugin.h
    TARGET ${plugin_TARGET}
  )

  add_library(${plugin_TARGET}
    id3libmetadataplugin.cpp
    mp3file.cpp
    ${plugin_GEN_MOC_SRCS})
  target_include_directories(${plugin_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(${plugin_TARGET} PRIVATE kid3-core Id3lib::Id3lib Kid3Plugin)

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif()

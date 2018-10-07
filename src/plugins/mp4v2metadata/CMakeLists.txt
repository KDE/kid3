if(WITH_MP4V2)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

  find_package(Mp4v2 REQUIRED)

  configure_file(mp4v2config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/mp4v2config.h)

  set(plugin_NAME Mp4v2Metadata)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  qt5_wrap_cpp(plugin_GEN_MOC_SRCS
    mp4v2metadataplugin.h
    TARGET ${plugin_TARGET}
  )

  add_library(${plugin_TARGET}
    mp4v2metadataplugin.cpp
    m4afile.cpp
    ${plugin_GEN_MOC_SRCS}
  )
  target_link_libraries(${plugin_TARGET} kid3-core Mp4v2::Mp4v2 Kid3Plugin)
  target_include_directories(${plugin_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif()

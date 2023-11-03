if(WITH_TAGLIB OR TAGLIB_LIBRARIES)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

  find_package(TagLib REQUIRED)

  set(plugin_NAME TaglibMetadata)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  add_library(${plugin_TARGET}
    taglibmetadataplugin.cpp
    taglibfile.cpp
  )
  if(${TAGLIB_VERSION} VERSION_LESS 2.0.0)
    target_sources(${plugin_TARGET} PRIVATE
      taglibext/aac/aacfiletyperesolver.cpp
      taglibext/mp2/mp2filetyperesolver.cpp
      taglibext/dsf/dsffiletyperesolver.cpp
      taglibext/dsf/dsffile.cpp
      taglibext/dsf/dsfproperties.cpp
      taglibext/dsf/dsfheader.cpp
      taglibext/dsdiff/dsdifffiletyperesolver.cpp
      taglibext/dsdiff/dsdifffile.cpp
      taglibext/dsdiff/dsdiffproperties.cpp
    )
  endif()
  if(NOT ${TAGLIB_VERSION} VERSION_GREATER 1.9.1)
    target_sources(${plugin_TARGET} PRIVATE
      taglibext/synchronizedlyricsframe.cpp
      taglibext/eventtimingcodesframe.cpp
    )
  endif()
  qt_wrap_cpp(plugin_GEN_MOC_SRCS
    taglibmetadataplugin.h
    TARGET ${plugin_TARGET}
  )
  target_sources(${plugin_TARGET} PRIVATE ${plugin_GEN_MOC_SRCS})
  target_include_directories(${plugin_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} taglibext)
  target_link_libraries(${plugin_TARGET} PRIVATE kid3-core TagLib::TagLib Kid3Plugin)

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif()

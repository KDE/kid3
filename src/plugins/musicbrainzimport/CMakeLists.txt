set(plugin_NAME MusicBrainzImport)

string(TOLOWER ${plugin_NAME} plugin_TARGET)

qt_wrap_cpp(plugin_GEN_MOC_SRCS
  musicbrainzimportplugin.h
  TARGET ${plugin_TARGET}
)

add_library(${plugin_TARGET}
  musicbrainzimportplugin.cpp
  musicbrainzimporter.cpp
  musicbrainzconfig.cpp
  ${plugin_GEN_MOC_SRCS}
)
target_link_libraries(${plugin_TARGET} kid3-core Kid3Plugin)

INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})

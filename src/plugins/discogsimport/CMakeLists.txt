set(plugin_NAME DiscogsImport)

string(TOLOWER ${plugin_NAME} plugin_TARGET)

qt5_wrap_cpp(plugin_GEN_MOC_SRCS
  discogsimportplugin.h
  TARGET ${plugin_TARGET}
)

add_library(${plugin_TARGET}
  discogsimportplugin.cpp
  discogsimporter.cpp
  discogsconfig.cpp
  ${plugin_GEN_MOC_SRCS}
)
target_link_libraries(${plugin_TARGET} kid3-core Kid3Plugin)

INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})

set(plugin_NAME AmazonImport)

string(TOLOWER ${plugin_NAME} plugin_TARGET)

qt_wrap_cpp(plugin_GEN_MOC_SRCS
  amazonimportplugin.h
  TARGET ${plugin_TARGET}
)

add_library(${plugin_TARGET}
  amazonimportplugin.cpp
  amazonimporter.cpp
  amazonconfig.cpp
  ${plugin_GEN_MOC_SRCS}
)
target_link_libraries(${plugin_TARGET} kid3-core Kid3Plugin)

INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})

set(plugin_NAME QmlCommand)

string(TOLOWER ${plugin_NAME} plugin_TARGET)

qt_wrap_cpp(plugin_GEN_MOC_SRCS
  qmlcommandplugin.h
  TARGET ${plugin_TARGET}
)

add_library(${plugin_TARGET}
  qmlcommandplugin.cpp
  ${plugin_GEN_MOC_SRCS}
)
target_link_libraries(${plugin_TARGET} kid3-core Qt${QT_VERSION_MAJOR}::Quick Kid3Plugin)

INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})

set(plugin_SRCS
  kid3appqmlplugin.cpp
  kid3qmlapplication.cpp
  qmlimageprovider.cpp
  frameobjectmodel.cpp
)

set(plugin_MOC_HDRS
  kid3appqmlplugin.h
  kid3qmlapplication.h
  frameobjectmodel.h
)

set(plugin_NAME Kid3AppQml)

if (WITH_GCC_PCH)
  add_definitions(${GCC_PCH_COMPILE_FLAGS})
endif (WITH_GCC_PCH)

string(TOLOWER ${plugin_NAME} plugin_TARGET)

qt4_wrap_cpp(plugin_GEN_MOC_SRCS ${plugin_MOC_HDRS})

add_library(${plugin_TARGET} MODULE ${plugin_SRCS} ${plugin_GEN_MOC_SRCS})
target_link_libraries(${plugin_TARGET} kid3-core ${BASIC_LIBRARIES})

install(TARGETS ${plugin_TARGET}
  LIBRARY DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3App
  RUNTIME DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3App)
set_target_properties(
  ${plugin_TARGET} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}/imports/Kid3App
  LIBRARY_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}/imports/Kid3App
)

add_custom_command(TARGET ${plugin_TARGET} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "module Kid3App" > "${kid3_plugins_BINARY_DIR}/imports/Kid3App/qmldir"
  COMMAND ${CMAKE_COMMAND} -E echo "plugin kid3appqml" >> "${kid3_plugins_BINARY_DIR}/imports/Kid3App/qmldir"
)
install(FILES "${kid3_plugins_BINARY_DIR}/imports/Kid3App/qmldir"
        DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3App)

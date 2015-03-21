set(plugin_SRCS
  kid3qmlplugin.cpp
  qmlimageprovider.cpp
  scriptutils.cpp
  configobjects.cpp
  checkablelistmodel.cpp
)

set(plugin_MOC_HDRS
  kid3qmlplugin.h
  scriptutils.h
  configobjects.h
  checkablelistmodel.h
)

set(plugin_QML_SRCS
  Kid3Script.qml
)

set(plugin_NAME Kid3Qml)

if (WITH_GCC_PCH)
  add_definitions(${GCC_PCH_COMPILE_FLAGS})
endif (WITH_GCC_PCH)

string(TOLOWER ${plugin_NAME} plugin_TARGET)

qt4_wrap_cpp(plugin_GEN_MOC_SRCS ${plugin_MOC_HDRS})

set(_targetDependencies ${plugin_SRCS} ${plugin_GEN_MOC_SRCS}
  ${plugin_QML_SRCS} "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml")
if (APPLE)
  # The extension must be dylib, not so.
  add_library(${plugin_TARGET} SHARED ${_targetDependencies})
else (APPLE)
  add_library(${plugin_TARGET} MODULE ${_targetDependencies})
endif (APPLE)
target_link_libraries(${plugin_TARGET} kid3-core ${BASIC_LIBRARIES})

install(TARGETS ${plugin_TARGET}
  LIBRARY DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3
  RUNTIME DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3)
set_target_properties(
  ${plugin_TARGET} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}/imports/Kid3
  LIBRARY_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}/imports/Kid3
)

if (WIN32)
  set(_qmldirPluginName libkid3qml)
else (WIN32)
  set(_qmldirPluginName kid3qml)
endif (WIN32)
add_custom_command(TARGET ${plugin_TARGET} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "module Kid3" > "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
  COMMAND ${CMAKE_COMMAND} -E echo "plugin ${_qmldirPluginName}" >> "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
  COMMAND ${CMAKE_COMMAND} -E echo "Kid3Script 1.0 Kid3Script.qml" >> "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
)
add_custom_command(OUTPUT "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Kid3Script.qml "${kid3_plugins_BINARY_DIR}/imports/Kid3/"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Kid3Script.qml
  WORKING_DIRECTORY "${kid3_plugins_BINARY_DIR}/imports/Kid3"
)

# Preprocess QML files for Qt 4 or Ubuntu Components.
if (NOT HAVE_QT5 OR WITH_UBUNTUCOMPONENTS)
  if (NOT HAVE_QT5)
    set(_qmlpp_code 4)
  else (NOT HAVE_QT5)
    set(_qmlpp_code U)
  endif (NOT HAVE_QT5)
  add_custom_command(OUTPUT "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
    COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qmlpp.pl ${_qmlpp_code}
    WORKING_DIRECTORY "${kid3_plugins_BINARY_DIR}/imports/Kid3"
    APPEND
  )
endif (NOT HAVE_QT5 OR WITH_UBUNTUCOMPONENTS)

install(FILES "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
              "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
        DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3)
if (NOT WIN32 AND HAVE_QT5)
  if (HAVE_QT5)
    get_target_property(_qmakeLocation Qt5::qmake LOCATION)
  else (HAVE_QT5)
    set(_qmakeLocation "${QT_QMAKE_EXECUTABLE}")
  endif (HAVE_QT5)
  execute_process(COMMAND "${_qmakeLocation}" -query QT_INSTALL_BINS
    OUTPUT_VARIABLE _qtInstallBins OUTPUT_STRIP_TRAILING_WHITESPACE)
  add_custom_command(OUTPUT "${kid3_plugins_BINARY_DIR}/imports/Kid3/plugins.qmltypes"
    COMMAND ${_qtInstallBins}/qmlplugindump -notrelocatable Kid3 1.0
      "${kid3_plugins_BINARY_DIR}/imports" > plugins.qmltypes
    DEPENDS ${plugin_TARGET}
    WORKING_DIRECTORY "${kid3_plugins_BINARY_DIR}/imports/Kid3")
  add_custom_target(plugins_qmltypes ALL
    DEPENDS "${kid3_plugins_BINARY_DIR}/imports/Kid3/plugins.qmltypes")
  add_dependencies(plugins_qmltypes ${plugin_TARGET})

  install(FILES "${kid3_plugins_BINARY_DIR}/imports/Kid3/plugins.qmltypes"
          DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3)
endif (NOT WIN32 AND HAVE_QT5)

if ((WIN32 OR APPLE) AND HAVE_QT5)
  # Install essential QML modules.
  get_target_property(QT_QML_DIR Qt5::qmake LOCATION)
  get_filename_component(QT_QML_DIR ${QT_QML_DIR} PATH)
  get_filename_component(QT_QML_DIR ${QT_QML_DIR} PATH)
  set(QT_QML_DIR ${QT_QML_DIR}/qml)
  install(DIRECTORY "${QT_QML_DIR}/QtQuick.2"
    DESTINATION "${WITH_PLUGINSDIR}/imports"
    FILES_MATCHING PATTERN "*plugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "qmldir")
  install(DIRECTORY "${QT_QML_DIR}/QtQuick/Layouts"
    DESTINATION "${WITH_PLUGINSDIR}/imports/QtQuick"
    FILES_MATCHING PATTERN "*plugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "qmldir")
endif ()

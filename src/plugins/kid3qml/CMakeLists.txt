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

qt5_wrap_cpp(plugin_GEN_MOC_SRCS ${plugin_MOC_HDRS})

set(_targetDependencies ${plugin_SRCS} ${plugin_GEN_MOC_SRCS}
  ${plugin_QML_SRCS} "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml")
if (APPLE)
  # The extension must be dylib, not so.
  add_library(${plugin_TARGET} SHARED ${_targetDependencies})
else (APPLE)
  add_library(${plugin_TARGET} MODULE ${_targetDependencies})
endif (APPLE)
target_link_libraries(${plugin_TARGET} kid3-core ${BASIC_LIBRARIES})

if (NOT HAVE_QMLDIR_IN_QRC)
  install(TARGETS ${plugin_TARGET}
    LIBRARY DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3
    RUNTIME DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3)
  set_target_properties(
    ${plugin_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}/imports/Kid3
    LIBRARY_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}/imports/Kid3
  )
else (NOT HAVE_QMLDIR_IN_QRC)
  install(TARGETS ${plugin_TARGET}
    LIBRARY DESTINATION ${WITH_BINDIR}
    RUNTIME DESTINATION ${WITH_BINDIR})
  file(MAKE_DIRECTORY "${kid3_plugins_BINARY_DIR}/imports/Kid3")
endif (NOT HAVE_QMLDIR_IN_QRC)
if (ANDROID)
  # Fix "-soname: must take a non-empty argument" error with CMake >= 3.2.
  # http://stackoverflow.com/questions/26841603/arm-linux-androideabi-bin-ld-fatal-error-soname-must-take-a-non-empty-argume
  string(REPLACE "<CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG><TARGET_SONAME>" ""
         CMAKE_CXX_CREATE_SHARED_MODULE "${CMAKE_CXX_CREATE_SHARED_MODULE}")
endif (ANDROID)
if (LINUX_SELF_CONTAINED)
  # Just to keep CMake quiet, not needed at run time.
  set_target_properties(
    ${plugin_TARGET} PROPERTIES INSTALL_RPATH "\$ORIGIN/../../..")
endif (LINUX_SELF_CONTAINED)

if (WIN32)
  set(_qmldirPluginName libkid3qml)
else (WIN32)
  set(_qmldirPluginName kid3qml)
endif (WIN32)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qmldir.in
               "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
               @ONLY)
add_custom_command(OUTPUT "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Kid3Script.qml "${kid3_plugins_BINARY_DIR}/imports/Kid3/"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Kid3Script.qml
  WORKING_DIRECTORY "${kid3_plugins_BINARY_DIR}/imports/Kid3"
)

install(FILES "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
              "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
        DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3)
if (NOT WIN32 AND NOT ANDROID AND NOT(CMAKE_CXX_COMPILER MATCHES "/osxcross/"))
  get_target_property(_qmakeLocation Qt5::qmake LOCATION)
  execute_process(COMMAND "${_qmakeLocation}" -query QT_INSTALL_BINS
    OUTPUT_VARIABLE _qtInstallBins OUTPUT_STRIP_TRAILING_WHITESPACE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runqmlplugindump.cmake.in
                 ${CMAKE_CURRENT_BINARY_DIR}/runqmlplugindump.cmake
                 @ONLY)
  add_custom_command(OUTPUT "${kid3_plugins_BINARY_DIR}/imports/Kid3/plugins.qmltypes"
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/runqmlplugindump.cmake
    DEPENDS ${plugin_TARGET}
    WORKING_DIRECTORY "${kid3_plugins_BINARY_DIR}/imports/Kid3")
  add_custom_target(plugins_qmltypes ALL
    DEPENDS "${kid3_plugins_BINARY_DIR}/imports/Kid3/plugins.qmltypes")
  add_dependencies(plugins_qmltypes ${plugin_TARGET})

  install(FILES "${kid3_plugins_BINARY_DIR}/imports/Kid3/plugins.qmltypes"
          DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3 OPTIONAL)
endif ()

if (WIN32 OR APPLE)
  # Install essential QML modules.
  get_target_property(QT_QML_DIR Qt5::qmake LOCATION)
  get_filename_component(QT_QML_DIR ${QT_QML_DIR} PATH)
  get_filename_component(QT_QML_DIR ${QT_QML_DIR} PATH)
  set(QT_QML_DIR ${QT_QML_DIR}/qml)
  install(DIRECTORY "${QT_QML_DIR}/QtQuick.2"
    DESTINATION "${WITH_PLUGINSDIR}/imports"
    FILES_MATCHING PATTERN "*plugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "qmldir"
                   PATTERN "*.dSYM" EXCLUDE)
  install(DIRECTORY "${QT_QML_DIR}/QtQuick/Layouts"
    DESTINATION "${WITH_PLUGINSDIR}/imports/QtQuick"
    FILES_MATCHING PATTERN "*plugin${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "qmldir"
                   PATTERN "*.dSYM" EXCLUDE)
endif ()

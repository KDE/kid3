set(plugin_NAME Kid3Qml)

string(TOLOWER ${plugin_NAME} plugin_TARGET)

if(APPLE)
  # The extension must be dylib, not so.
  add_library(${plugin_TARGET} SHARED kid3qmlplugin.cpp)
else()
  add_library(${plugin_TARGET} MODULE kid3qmlplugin.cpp)
endif()

qt5_wrap_cpp(plugin_GEN_MOC_SRCS
  kid3qmlplugin.h
  scriptutils.h
  configobjects.h
  checkablelistmodel.h
  TARGET ${plugin_TARGET}
)

target_sources(${plugin_TARGET} PRIVATE
  qmlimageprovider.cpp
  scriptutils.cpp
  configobjects.cpp
  checkablelistmodel.cpp
  ${plugin_GEN_MOC_SRCS}
  Kid3Script.qml
  "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
)

target_link_libraries(${plugin_TARGET} kid3-core kid3-gui Qt5::Quick Kid3Plugin)

if(NOT HAVE_QMLDIR_IN_QRC)
  if(APPLE)
    # Folders containing a dot will cause Apple's code signing to fail.
    # In order to still have QtQuick.2 available, the QML plugins are put into
    # the Resources folder. But binary code in Resources is prohibited,
    # therefore the libraries are in the Plugins folder with symlinks from
    # the QML imports folders.
    # https://www.qt.io/blog/2014/10/29/an-update-on-os-x-code-signing
    install(TARGETS ${plugin_TARGET}
      LIBRARY DESTINATION ${WITH_PLUGINSDIR}
      RUNTIME DESTINATION ${WITH_PLUGINSDIR})
  else()
    install(TARGETS ${plugin_TARGET}
      LIBRARY DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3
      RUNTIME DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3)
    set_target_properties(
      ${plugin_TARGET} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}/imports/Kid3
      LIBRARY_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}/imports/Kid3
    )
  endif()
else()
  install(TARGETS ${plugin_TARGET}
    LIBRARY DESTINATION ${WITH_BINDIR}
    RUNTIME DESTINATION ${WITH_BINDIR})
  file(MAKE_DIRECTORY "${kid3_plugins_BINARY_DIR}/imports/Kid3")
endif()
if(ANDROID)
  # Fix "-soname: must take a non-empty argument" error with CMake >= 3.2.
  # https://stackoverflow.com/questions/26841603/arm-linux-androideabi-bin-ld-fatal-error-soname-must-take-a-non-empty-argume
  string(REPLACE "<CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG><TARGET_SONAME>" ""
         CMAKE_CXX_CREATE_SHARED_MODULE "${CMAKE_CXX_CREATE_SHARED_MODULE}")
endif()
if(LINUX_SELF_CONTAINED)
  # Just to keep CMake quiet, not needed at run time.
  set_target_properties(
    ${plugin_TARGET} PROPERTIES INSTALL_RPATH "\$ORIGIN/../../..")
endif()

if(WIN32 AND NOT MSVC)
  set(_qmldirPluginName libkid3qml)
else()
  set(_qmldirPluginName kid3qml)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qmldir.in
               "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
               @ONLY)
add_custom_command(OUTPUT "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Kid3Script.qml "${kid3_plugins_BINARY_DIR}/imports/Kid3/"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Kid3Script.qml
  WORKING_DIRECTORY "${kid3_plugins_BINARY_DIR}/imports/Kid3"
)

if(APPLE)
  install(FILES "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
                "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
          DESTINATION ${WITH_QMLDIR}/imports/Kid3)
else()
  install(FILES "${kid3_plugins_BINARY_DIR}/imports/Kid3/qmldir"
                "${kid3_plugins_BINARY_DIR}/imports/Kid3/Kid3Script.qml"
          DESTINATION ${WITH_PLUGINSDIR}/imports/Kid3)
endif()
if(NOT WIN32 AND NOT ANDROID AND NOT APPLE)
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
endif()

if(WIN32 OR APPLE OR LINUX_SELF_CONTAINED)
  # Install essential QML modules.
  get_target_property(QT_QML_DIR Qt5::qmake LOCATION)
  get_filename_component(QT_QML_DIR ${QT_QML_DIR} PATH)
  get_filename_component(QT_QML_DIR ${QT_QML_DIR} PATH)
  set(QT_QML_DIR ${QT_QML_DIR}/qml)
  if(APPLE)
    install(FILES "${QT_QML_DIR}/QtQuick.2/qmldir"
      DESTINATION "${WITH_QMLDIR}/imports/QtQuick.2")
    install(FILES "${QT_QML_DIR}/QtQuick/Layouts/qmldir"
      DESTINATION "${WITH_QMLDIR}/imports/QtQuick/Layouts")
    install(FILES "${QT_QML_DIR}/QtQuick.2/libqtquick2plugin.dylib"
                  "${QT_QML_DIR}/QtQuick/Layouts/libqquicklayoutsplugin.dylib"
      DESTINATION "${WITH_PLUGINSDIR}")
    install(CODE
      "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        \"../../../../PlugIns/libkid3qml.dylib\"
        \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_QMLDIR}/imports/Kid3/libkid3qml.dylib\")
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        \"../../../../PlugIns/libqtquick2plugin.dylib\"
        \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_QMLDIR}/imports/QtQuick.2/libqtquick2plugin.dylib\")
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        \"../../../../../PlugIns/libqquicklayoutsplugin.dylib\"
        \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_QMLDIR}/imports/QtQuick/Layouts/libqquicklayoutsplugin.dylib\")
      ")
  else()
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
  endif()
endif()

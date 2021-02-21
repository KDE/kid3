if(NOT ANDROID)
  add_executable(kid3-qml MACOSX_BUNDLE WIN32 mainqml.cpp)
  install(TARGETS kid3-qml
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${WITH_BINDIR})
else()
  add_library(kid3-qml SHARED mainqml.cpp)
  # Make symbols visible so that main can be found.
  set_target_properties(kid3-qml PROPERTIES COMPILE_FLAGS -fvisibility=default)
endif()

if(HAVE_QMLDIR_IN_QRC)
  qt_add_resources(qmlapp_GEN_RCS qmlapp.qrc)
  target_sources(kid3-qml PRIVATE ${qmlapp_GEN_RCS})
endif()

if(HAVE_TRANSLATIONSDIR_IN_QRC)
  string(REPLACE ":/" "/" _qrcPrefix ${WITH_TRANSLATIONSDIR})
  add_custom_command(
    OUTPUT translations.qrc
    COMMAND ${CMAKE_COMMAND} -P
      ${CMAKE_CURRENT_SOURCE_DIR}/gentranslationsqrc.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
      ${CMAKE_CURRENT_BINARY_DIR}/../../../translations ${_qrcPrefix}
    DEPENDS translations
  )
  qt_add_resources(
    translations_GEN_RCS ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)
  target_sources(kid3-qml PRIVATE ${translations_GEN_RCS})
endif()

target_link_libraries(kid3-qml kid3-gui Qt${QT_VERSION_MAJOR}::QuickControls2)
if(NOT MSVC)
  target_link_libraries(kid3-qml -lstdc++)
endif()

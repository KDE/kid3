set(qml_SRCS
  mainqml.cpp
)

if (HAVE_QMLDIR_IN_QRC)
  qt4_add_resources(qmlapp_GEN_RCS qmlapp.qrc)
else (HAVE_QMLDIR_IN_QRC)
  set(qmlapp_GEN_RCS)
endif (HAVE_QMLDIR_IN_QRC)

if (HAVE_TRANSLATIONSDIR_IN_QRC)
  string(REPLACE ":/" "/" _qrcPrefix ${WITH_TRANSLATIONSDIR})
  add_custom_command(
    OUTPUT translations.qrc
    COMMAND ${CMAKE_COMMAND} -P
      ${CMAKE_CURRENT_SOURCE_DIR}/gentranslationsqrc.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
      ${CMAKE_CURRENT_BINARY_DIR}/../../../po ${_qrcPrefix}
    DEPENDS translations
  )
  qt4_add_resources(
    translations_GEN_RCS ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)
else (HAVE_TRANSLATIONSDIR_IN_QRC)
  set(translations_GEN_RCS)
endif (HAVE_TRANSLATIONSDIR_IN_QRC)

add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
if (NOT ANDROID)
  add_executable(kid3-qml MACOSX_BUNDLE WIN32 ${qml_SRCS} ${qmlapp_GEN_RCS} ${translations_GEN_RCS})

  target_link_libraries(kid3-qml kid3-gui -lstdc++)

  install(TARGETS kid3-qml
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${WITH_BINDIR})
else (NOT ANDROID)
  add_library(kid3-qml SHARED ${qml_SRCS} ${qmlapp_GEN_RCS} ${translations_GEN_RCS})
  # Make symbols visible so that main can be found.
  set_target_properties(kid3-qml PROPERTIES COMPILE_FLAGS -fvisibility=default)

  target_link_libraries(kid3-qml kid3-gui -lstdc++)
endif (NOT ANDROID)

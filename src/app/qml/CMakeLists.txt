set(qml_SRCS
  mainqml.cpp
)

if (HAVE_QMLDIR_IN_QRC)
  set(qmlapp_RESOURCES qmlapp.qrc)
  qt4_add_resources(qmlapp_GEN_RCS ${qmlapp_RESOURCES})
else (HAVE_QMLDIR_IN_QRC)
  set(qmlapp_GEN_RCS)
endif (HAVE_QMLDIR_IN_QRC)

add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
if (NOT ANDROID)
  add_executable(kid3-qml MACOSX_BUNDLE WIN32 ${qml_SRCS} ${qmlapp_GEN_RCS})

  target_link_libraries(kid3-qml kid3-gui -lstdc++)

  install(TARGETS kid3-qml
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${WITH_BINDIR})
else (NOT ANDROID)
  add_library(kid3-qml SHARED ${qml_SRCS} ${qmlapp_GEN_RCS})
  # Make symbols visible so that main can be found.
  set_target_properties(kid3-qml PROPERTIES COMPILE_FLAGS -fvisibility=default)

  target_link_libraries(kid3-qml kid3-gui -lstdc++)
endif (NOT ANDROID)

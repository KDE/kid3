# Before starting Qt Creator, export the environment variable
# QML_IMPORT_PATH=<qt5dir>/qml:<qt5dir>/imports
# so that the editor support the QML types.
# On Linux, this can be done in ~/.xsessionrc.
set(qml_SRCS
  mainqml.cpp
)
set(qml_QML_SRCS
  ubuntu/main.qml
  ubuntu/Collapsible.qml
  ubuntu/ComboBox.qml
  ubuntu/DropDownList.qml
  ubuntu/FrameDelegate.qml
  ubuntu/FrameEditDialog.qml
  ubuntu/FrameSelectDialog.qml
  ubuntu/MessageDialog.qml
)
set(qml_MOC_HDRS
)
qt4_wrap_cpp(qml_GEN_MOC_SRCS ${qml_MOC_HDRS})
set(qml_SRCS ${qml_SRCS} ${qml_GEN_MOC_SRCS})

add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
set(qml_RESOURCES )
qt4_add_resources(qml_GEN_RCS ${qml_RESOURCES})
add_executable(kid3-qml MACOSX_BUNDLE WIN32
  ${qml_SRCS} ${qml_GEN_RCS} ${qml_QML_SRCS})

target_link_libraries(kid3-qml kid3-gui -lstdc++)

install(TARGETS kid3-qml
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${WITH_BINDIR})

install(FILES ${qml_QML_SRCS} DESTINATION ${WITH_QMLDIR})

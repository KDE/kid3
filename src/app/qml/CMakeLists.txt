# Before starting Qt Creator, export the environment variable
# QML_IMPORT_PATH=<qt5dir>/qml:<qt5dir>/imports
# so that the editor support the QML types.
# On Linux, this can be done in ~/.xsessionrc.
set(qml_SRCS
  mainqml.cpp
)
set(qml_QML_SRCS
  main.qml
  MainPage.qml
  Collapsible.qml
  ComboBox.qml
  DropDownList.qml
  FrameDelegate.qml
  FrameEditDialog.qml
  FrameSelectDialog.qml
  MessageDialog.qml
  ActionSelectionDelegate.qml
  UiConstants.qml
  FileList.qml
  FileCollapsible.qml
  Tag1Collapsible.qml
  Tag2Collapsible.qml
  PictureCollapsible.qml
  FileSelectDialog.qml
  RenameDirectoryPage.qml
  NumberTracksDialog.qml
  FilterPage.qml
  BatchImportPage.qml
  AbstractSettingsPage.qml
  SettingsElement.qml
  SettingsPage.qml
  RaisableRectangle.qml
  ComponentsQtQuick/ActionList.qml
  ComponentsQtQuick/Action.qml
  ComponentsQtQuick/ActionSelectionPopover.qml
  ComponentsQtQuick/Button.qml
  ComponentsQtQuick/CheckBox.qml
  ComponentsQtQuick/Dialog.qml
  ComponentsQtQuick/Empty.qml
  ComponentsQtQuick/Label.qml
  ComponentsQtQuick/MainView.qml
  ComponentsQtQuick/Page.qml
  ComponentsQtQuick/PageStack.qml
  ComponentsQtQuick/PopupBase.qml
  ComponentsQtQuick/Standard.qml
  ComponentsQtQuick/TextArea.qml
  ComponentsQtQuick/TextField.qml
  ComponentsQtQuick/ThinDivider.qml
)
set(qml_MOC_HDRS
)
qt4_wrap_cpp(qml_GEN_MOC_SRCS ${qml_MOC_HDRS})
set(qml_SRCS ${qml_SRCS} ${qml_GEN_MOC_SRCS})

add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
add_executable(kid3-qml MACOSX_BUNDLE WIN32
  ${qml_SRCS} ${qml_GEN_RCS} ${qml_QML_SRCS})

target_link_libraries(kid3-qml kid3-gui -lstdc++)

install(TARGETS kid3-qml
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${WITH_BINDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${WITH_QMLDIR}
        FILES_MATCHING PATTERN "*.qml" PATTERN "*.js" PATTERN "*.png")

# Preprocess installed QML files for Qt 4 or Ubuntu Components.
if (NOT HAVE_QT5 OR WITH_UBUNTUCOMPONENTS)
  if (NOT HAVE_QT5)
    set(_qmlpp_code 4)
  else (NOT HAVE_QT5)
    set(_qmlpp_code U)
  endif (NOT HAVE_QT5)
  install(CODE "
    execute_process(
      COMMAND ${PERL_EXECUTABLE}
        ${kid3_SOURCE_DIR}/src/plugins/kid3qml/qmlpp.pl ${_qmlpp_code}
      WORKING_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_QMLDIR}\"
    )
    ")
endif (NOT HAVE_QT5 OR WITH_UBUNTUCOMPONENTS)

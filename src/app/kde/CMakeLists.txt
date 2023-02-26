add_executable(kid3
  mainkde.cpp
  kdeplatformtools.cpp
  kdemainwindow.cpp
  kdeconfigdialog.cpp
  kdesettings.cpp
)

target_link_libraries(kid3 kid3-gui KF${QT_MAJOR_VERSION}::ConfigCore KF${QT_MAJOR_VERSION}::ConfigGui KF${QT_MAJOR_VERSION}::ConfigWidgets KF${QT_MAJOR_VERSION}::CoreAddons KF${QT_MAJOR_VERSION}::WidgetsAddons KF${QT_MAJOR_VERSION}::XmlGui KF${QT_MAJOR_VERSION}::KIOCore)
if(NOT MSVC)
  target_link_libraries(kid3 -lstdc++)
endif()

install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
install(PROGRAMS ../org.kde.kid3.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ../org.kde.kid3.appdata.xml DESTINATION "${WITH_DATAROOTDIR}/metainfo")
if(QT_VERSION_MAJOR EQUAL 6)
  # TODO KF6: If there something defined?
  if(NOT KXMLGUI_INSTALL_DIR)
    set(KXMLGUI_INSTALL_DIR "${WITH_DATAROOTDIR}/kxmlgui6")
  endif()
endif()
install(FILES kid3ui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kid3)

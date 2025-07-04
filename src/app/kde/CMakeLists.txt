if(POLICY CMP0177)
  cmake_policy(SET CMP0177 NEW)
endif()

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

install(PROGRAMS ../org.kde.kid3.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ../org.kde.kid3.appdata.xml DESTINATION "${WITH_DATAROOTDIR}/metainfo")
if(QT_VERSION_MAJOR EQUAL 6)
  install(TARGETS kid3 DESTINATION ${KDE_INSTALL_BINDIR})
  install(FILES kid3ui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/kid3)
else()
  install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
  install(FILES kid3ui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kid3)
endif()

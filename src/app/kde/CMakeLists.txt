add_executable(kid3
  mainkde.cpp
  kdeplatformtools.cpp
  kdemainwindow.cpp
  kdeconfigdialog.cpp
  kdesettings.cpp
)

target_link_libraries(kid3 kid3-gui KF5::ConfigCore KF5::ConfigGui KF5::ConfigWidgets KF5::CoreAddons KF5::WidgetsAddons KF5::XmlGui KF5::KIOCore -lstdc++)

install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
install(FILES ../org.kde.kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES ../org.kde.kid3.appdata.xml DESTINATION "${WITH_DATAROOTDIR}/metainfo")
install(FILES kid3ui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kid3)

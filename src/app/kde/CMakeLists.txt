set(kde_SRCS
  mainkde.cpp
  kdeplatformtools.cpp
  kdemainwindow.cpp
  kdeconfigdialog.cpp
  kdesettings.cpp
)

add_executable(kid3 ${kde_SRCS})

target_link_libraries(kid3 kid3-gui KF5::ConfigCore KF5::ConfigGui KF5::ConfigWidgets KF5::CoreAddons KF5::WidgetsAddons KF5::XmlGui -lstdc++)

install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
install(FILES ../kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR}
                              RENAME net.sourceforge.kid3.desktop)
install(FILES ../kid3.appdata.xml DESTINATION "${WITH_DATAROOTDIR}/metainfo"
                                  RENAME net.sourceforge.kid3.appdata.xml)
install(FILES kid3ui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kid3)

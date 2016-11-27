set(kde_SRCS
  mainkde.cpp
  kdeplatformtools.cpp
  kdemainwindow.cpp
  kdeconfigdialog.cpp
  kdesettings.cpp
)

if (HAVE_QT5)
  if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
  endif (POLICY CMP0063)

  add_executable(kid3 ${kde_SRCS})

  target_link_libraries(kid3 kid3-gui KF5::ConfigCore KF5::ConfigGui KF5::ConfigWidgets KF5::CoreAddons KF5::WidgetsAddons KF5::XmlGui -lstdc++)

  install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
  install(FILES ../kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
  install(FILES ../kid3.appdata.xml DESTINATION "${WITH_DATAROOTDIR}/metainfo")
  install(FILES kid3ui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kid3)
else (HAVE_QT5)
  add_definitions(${KDE4_DEFINITIONS} -DKDE_NO_COMPAT ${KDE4_ENABLE_EXCEPTIONS})
  include_directories(${KDE4_INCLUDES})

  kde4_add_executable(kid3 ${kde_SRCS})

  target_link_libraries(kid3 kid3-gui ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} -lstdc++)

  install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
  install(FILES ../kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
  install(FILES ../kid3.appdata.xml DESTINATION "${WITH_DATAROOTDIR}/metainfo")
  install(FILES kid3ui.rc DESTINATION ${DATA_INSTALL_DIR}/kid3)
endif (HAVE_QT5)

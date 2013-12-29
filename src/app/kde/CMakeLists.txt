set(kde_SRCS
  mainkde.cpp
  kdeplatformtools.cpp
  kdemainwindow.cpp
  kdeconfigdialog.cpp
  kdesettings.cpp
)

add_definitions(${KDE4_DEFINITIONS} -DKDE_NO_COMPAT)
include_directories(${KDE4_INCLUDES})

kde4_add_executable(kid3 ${kde_SRCS})

target_link_libraries(kid3 kid3-gui ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} -lstdc++)

install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
install(FILES ../kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kid3ui.rc DESTINATION ${DATA_INSTALL_DIR}/kid3)

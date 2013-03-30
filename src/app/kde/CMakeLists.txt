set(kde_SRCS mainkde.cpp kdeplatformtools.cpp)

kde4_add_executable(kid3 ${kde_SRCS})

target_link_libraries(kid3 kid3-gui)

install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
install(FILES ../kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kid3ui.rc DESTINATION ${DATA_INSTALL_DIR}/kid3)

kde4_install_icons(${ICON_INSTALL_DIR})

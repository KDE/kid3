include_directories(
  ../core/config
  ../core/utils
  ../core/tags
  ../core/model
  ../core/import
  ../gui/forms
  ../gui/dialogs
)

if (BUILD_QT_APP)
  add_subdirectory(qt)
endif (BUILD_QT_APP)

add_subdirectory(cli)

if (BUILD_KDE_APP)
  find_package(KDE4 REQUIRED)
  include (KDE4Defaults)
  add_subdirectory(kde)
  kde4_install_icons(${ICON_INSTALL_DIR})
endif (BUILD_KDE_APP)

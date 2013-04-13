if (BUILD_KDE_APP)
  find_package(KDE4 REQUIRED)
  kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR kid3)
endif (BUILD_KDE_APP)

if (BUILD_QT_APP)
  kid3_create_handbook(index.docbook)
endif (BUILD_QT_APP)

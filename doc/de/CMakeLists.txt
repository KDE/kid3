if (BUILD_KDE_APP)
  kf5_create_handbook(index.docbook)
endif (BUILD_KDE_APP)

if (BUILD_QT_APP)
  kid3_create_handbook(index.docbook)
endif (BUILD_QT_APP)

if (UNIX)
  kid3_create_manpage(index.docbook)
endif (UNIX)

if (WITH_KDE)

kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/de SUBDIR kid3)

else (WITH_KDE)

kid3_create_handbook(index.docbook)

endif (WITH_KDE)

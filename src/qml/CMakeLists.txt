if (NOT HAVE_QMLDIR_IN_QRC)
# Before starting Qt Creator, export the environment variable
# QML_IMPORT_PATH=<qt5dir>/qml:<qt5dir>/imports
# so that the editor supports the QML types.
# On Linux, this can be done in ~/.xsessionrc.
set(qml_SUBDIRS script)
if (BUILD_QML_APP)
  set(qml_SUBDIRS ${qml_SUBDIRS} app)
endif (BUILD_QML_APP)

set(qml_QML_SRCS)
foreach (_subdir ${qml_SUBDIRS})
  include(${_subdir}/Sources.cmake)
  set(qml_QML_SRCS ${qml_QML_SRCS} ${${_subdir}_QML_SRCS})
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_subdir}
          DESTINATION ${WITH_QMLDIR}
          FILES_MATCHING PATTERN "*.qml" PATTERN "*.js")
endforeach (_subdir)
if (BUILD_QML_APP)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/icons
          DESTINATION ${WITH_QMLDIR}
          FILES_MATCHING PATTERN "*.png" PATTERN "*.svg")
endif (BUILD_QML_APP)

# Only used to add the QML sources to the project.
add_custom_target(qml-sources SOURCES ${qml_QML_SRCS})

endif (NOT HAVE_QMLDIR_IN_QRC)

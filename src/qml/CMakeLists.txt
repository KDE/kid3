if(POLICY CMP0177)
  cmake_policy(SET CMP0177 NEW)
endif()

# Make sure to keep this in sync with qmlapp.qrc
set(QMLAPP_QML_SOURCES
  app/AboutDialog.qml
  app/AbstractSettingsPage.qml
  app/BatchImportPage.qml
  app/Collapsible.qml
  app/Empty.qml
  app/FileCollapsible.qml
  app/FileList.qml
  app/FileSelectDialog.qml
  app/FilterPage.qml
  app/FrameDelegate.qml
  app/FrameEditDialog.qml
  app/FrameSelectDialog.qml
  app/IconButton.qml
  app/ImportProfilesEditPage.qml
  app/Main.qml
  app/MainPage.qml
  app/MapEditPage.qml
  app/MessageDialog.qml
  app/NumberTracksDialog.qml
  app/PictureCollapsible.qml
  app/RenameDirectoryPage.qml
  app/ScaledImage.qml
  app/SettingsElement.qml
  app/SettingsItem.qml
  app/SettingsPage.qml
  app/Standard.qml
  app/StringListEditPage.qml
  app/TagCollapsible.qml
  app/ThinDivider.qml
  app/UiConstants.qml
  CACHE INTERNAL ""
)
if(NOT HAVE_QMLDIR_IN_QRC)
  # Before starting Qt Creator, export the environment variable
  # QML_IMPORT_PATH=<qtdir>/qml:<qtdir>/imports
  # so that the editor supports the QML types.
  # On Linux, this can be done in ~/.xsessionrc.
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/script
          DESTINATION ${WITH_QMLDIR}
          FILES_MATCHING PATTERN "*.qml" PATTERN "*.js")

  if(BUILD_QML_APP)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/app
            DESTINATION ${WITH_QMLDIR}
            FILES_MATCHING PATTERN "*.qml" PATTERN "*.js")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/icons
            DESTINATION ${WITH_QMLDIR}
            FILES_MATCHING PATTERN "*.ttf" PATTERN "triangle_*.svg")
  endif()

  # Only used to add the QML sources to the project.
  add_custom_target(qml-sources SOURCES
    script/ReplayGain2SoundCheck.qml
    script/ResizeAlbumArt.qml
    script/QmlConsole.qml
    script/ShowTextEncodingV1.qml
    script/ExportCsv.qml
    script/ImportCsv.qml
    script/ExportJson.qml
    script/ImportJson.qml
    script/ExtractAlbumArt.qml
    script/EmbedAlbumArt.qml
    script/EmbedLyrics.qml
    script/ExportPlaylist.qml
    script/TitleCase.qml
    script/Tag1ToAscii.qml
    script/RewriteTags.qml
    script/FixId3v2StandardViolations.qml
  )
  if(BUILD_QML_APP)
    # Only used to add the QML sources to the project.
    add_custom_target(qmlapp-sources SOURCES ${QMLAPP_QML_SOURCES})
  endif()
endif()

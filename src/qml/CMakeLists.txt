if(NOT HAVE_QMLDIR_IN_QRC)
  # Before starting Qt Creator, export the environment variable
  # QML_IMPORT_PATH=<qt5dir>/qml:<qt5dir>/imports
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
    script/ExtractAlbumArt.qml
    script/EmbedAlbumArt.qml
    script/EmbedLyrics.qml
    script/ExportPlaylist.qml
  )
  if(BUILD_QML_APP)
    # Only used to add the QML sources to the project.
    add_custom_target(qmlapp-sources SOURCES
      app/Main.qml
      app/MainPage.qml
      app/Collapsible.qml
      app/FrameDelegate.qml
      app/FrameEditDialog.qml
      app/FrameSelectDialog.qml
      app/MessageDialog.qml
      app/UiConstants.qml
      app/FileList.qml
      app/FileCollapsible.qml
      app/TagCollapsible.qml
      app/PictureCollapsible.qml
      app/FileSelectDialog.qml
      app/RenameDirectoryPage.qml
      app/NumberTracksDialog.qml
      app/FilterPage.qml
      app/BatchImportPage.qml
      app/AbstractSettingsPage.qml
      app/SettingsElement.qml
      app/SettingsPage.qml
      app/SettingsItem.qml
      app/AboutDialog.qml
      app/Empty.qml
      app/Standard.qml
      app/ThinDivider.qml
      app/IconButton.qml
      app/ThinDivider.qml
      app/StringListEditPage.qml
      app/MapEditPage.qml
      app/ImportProfilesEditPage.qml
    )
  endif()  
endif()

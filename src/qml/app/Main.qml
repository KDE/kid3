/**
 * \file Main.qml
 * Main entry point for QML application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import "../componentsqtquick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3 1.0

MainView {
  id: root
  objectName: "mainView"
  applicationName: "Kid3"
  useDeprecatedToolbar: false
  automaticOrientation: true
  width: constants.gu(100)
  height: constants.gu(100)

  UiConstants {
    id: constants
  }

  FrameEditorObject {
    id: frameEditor
  }

  ScriptUtils {
    id: script
  }

  ConfigObjects {
    id: configs
  }

  FrameSelectDialog {
    id: frameSelectDialog

    onFrameSelected: frameEditor.onFrameSelectionFinished(name)

    Connections {
      target: frameEditor
      onFrameSelectionRequested: frameSelectDialog.open(frameNames)
    }
  }

  FrameEditDialog {
    id: frameEditDialog
    onFrameEdited: frameEditor.onFrameEditFinished(frame)

    Connections {
      target: frameEditor
      onFrameEditRequested: frameEditDialog.open(frame)
    }
  }

  MessageDialog {
    property bool doNotRevert: false

    signal completed(bool ok)

    id: saveModifiedDialog
    title: qsTr("Warning")
    text: qsTr("The current directory has been modified.\n" +
               "Do you want to save it?")
    onYes: {
      app.saveDirectory()
      completed(true)
    }
    onNo: {
      if (!doNotRevert) {
        app.deselectAllFiles()
        app.revertFileModifications()
      }
      completed(true)
    }
    onRejected: completed(false)

    // Open dialog if any file modified.
    // completed(ok) is signalled with false if canceled.
    function openIfModified() {
      if (app.modified && app.dirName) {
        show()
      } else {
        completed(true)
      }
    }
  }

  NumberTracksDialog {
    id: numberTracksDialog
  }

  AboutDialog {
    id: aboutDialog
  }

  Component {
    id: mainMenuPopoverComponent
    ActionSelectionPopover {
      id: mainMenuPopover
      delegate: ActionSelectionDelegate {
        popover: mainMenuPopover
      }
      actions: ActionList {
        Action {
          text: qsTr("About")
          onTriggered: aboutDialog.show()
        }
        Action {
          text: qsTr("Save")
          onTriggered: {
            var errorFiles = app.saveDirectory()
            if (errorFiles.length > 0) {
              console.debug("Save error:" + errorFiles)
            }
          }
        }
        Action {
          text: qsTr("Settings")
          onTriggered: pageStack.push(settingsPage)
        }
        Action {
          text: qsTr("Automatic Import")
          onTriggered: pageStack.push(batchImportPage)
        }
        Action {
          text: qsTr("Create Playlist")
          onTriggered: app.writePlaylist()
        }
        Action {
          text: qsTr("Rename Directory")
          onTriggered: pageStack.push(renameDirPage)
        }
        Action {
          text: qsTr("Number Tracks")
          onTriggered: numberTracksDialog.show()
        }
        Action {
          text: qsTr("Filter")
          onTriggered: pageStack.push(filterPage)
        }
        Action {
          text: qsTr("Apply Filename Format")
          onTriggered: app.applyFilenameFormat()
        }
        Action {
          text: qsTr("Apply Tag Format")
          onTriggered: app.applyTagFormat()
        }
        Action {
          text: qsTr("Apply Text Encoding")
          onTriggered: app.applyTextEncoding()
        }
        Action {
          text: qsTr("Convert ID3v2.3 to ID3v2.4")
          onTriggered: app.convertToId3v24()
        }
        Action {
          text: qsTr("Convert ID3v2.4 to ID3v2.3")
          onTriggered: app.convertToId3v23()
        }
        Action {
          text: qsTr("Revert")
          onTriggered: app.revertFileModifications()
        }
        Action {
          text: qsTr("Quit")
          onTriggered: confirmedQuit()
        }
      }
    }
  }

  PageStack {
    id: pageStack
    onBackOnLastPagePressed: confirmedQuit() //@!Ubuntu
    Component.onCompleted: push(mainPage)
    MainPage {
      id: mainPage

      Component {
        id: openDialog
        FileSelectDialog {
          property variant field
          parent: root
          title: qsTr("Open")
          onFinished: if (path) confirmedOpenDirectory(path)
        }
      }

      visible: false
      onTitlePressed: constants.openPopup(openDialog, root,          //@!Ubuntu
                                          {"filePath": app.dirName}) //@!Ubuntu
    }
    RenameDirectoryPage {
      id: renameDirPage
      visible: false
    }
    FilterPage {
      id: filterPage
      visible: false
    }
    BatchImportPage {
      id: batchImportPage
      visible: false
    }
    SettingsPage {
      id: settingsPage
      visible: false
    }
  }

  Text {                                                        //@!Ubuntu
    visible: false                                              //@!Ubuntu
    Component.onCompleted: {                                    //@!Ubuntu
      // Linux Desktop: pixelSize 12 => gu = 8
      // Android Emulator Galaxy Nexus: 32 => gu = 21
      constants.gridUnit = Math.max(8 * font.pixelSize / 12, 8) //@!Ubuntu
      constants.titlePixelSize = 18 * font.pixelSize / 12       //@!Ubuntu
      constants.imageScaleFactor = Math.max(font.pixelSize / 12.0, 1.0) //@!Ubuntu
    }                                                           //@!Ubuntu
  }                                                             //@!Ubuntu

  Component.onCompleted: {
    app.frameEditor = frameEditor
    app.readConfig()
    app.openDirectory(configs.fileConfig().lastOpenedFile)
  }

  Component.onDestruction: {
    app.frameEditor = null
  }

  Connections {
    target: app

    onConfirmedOpenDirectoryRequested: confirmedOpenDirectory(paths)
    onFileSelectionUpdateRequested: mainPage.updateCurrentSelection()
    onSelectedFilesUpdated: app.tagsToFrameModels()
    onSelectedFilesChanged: app.tagsToFrameModels()
  }
  Connections {
    target: app.downloadClient
    onDownloadFinished: app.imageDownloaded(data, contentType, url)
  }

  DropArea {                        //@QtQuick2
    anchors.fill: parent            //@QtQuick2
    onDropped: {                    //@QtQuick2
      if (drop.hasUrls) {           //@QtQuick2
        app.openDropUrls(drop.urls) //@QtQuick2
      }                             //@QtQuick2
    }                               //@QtQuick2
  }                                 //@QtQuick2

  function confirmedOpenDirectory(path) {
    function openIfCompleted(ok) {
      saveModifiedDialog.completed.disconnect(openIfCompleted)
      if (ok) {
        app.openDirectory(path)
      }
    }

    mainPage.updateCurrentSelection()
    saveModifiedDialog.doNotRevert = false
    saveModifiedDialog.completed.connect(openIfCompleted)
    saveModifiedDialog.openIfModified()
  }

  function confirmedQuit() {
    mainPage.updateCurrentSelection()
    saveModifiedDialog.doNotRevert = true
    saveModifiedDialog.completed.connect(quitIfCompleted)
    saveModifiedDialog.openIfModified()
  }

  function quitIfCompleted(ok) {
    saveModifiedDialog.completed.disconnect(quitIfCompleted)
    if (ok) {
      var currentFile = mainPage.currentFilePath()
      if (currentFile) {
        configs.fileConfig().lastOpenedFile = currentFile
      }
      app.saveConfig()
      Qt.quit()
    }
  }
}

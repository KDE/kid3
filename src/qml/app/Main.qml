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

  Component {
    id: mainMenuPopoverComponent
    ActionSelectionPopover {
      id: mainMenuPopover
      delegate: ActionSelectionDelegate {
        popover: mainMenuPopover
      }
      actions: ActionList {
        Action {
          text: qsTr("Settings")
          onTriggered: pageStack.push(settingsPage)
        }
        Action {
          text: qsTr("Apply Filename Format")
          onTriggered: app.applyFilenameFormat()
        }
        Action {
          text: qsTr("Apply Tag Format")
          onTriggered: app.applyId3Format()
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
          text: qsTr("Automatic Import")
          onTriggered: pageStack.push(batchImportPage)
        }
        Action {
          text: qsTr("Create Playlist")
          onTriggered: app.writePlaylist()
        }
        Action {
          text: qsTr("Revert")
          onTriggered: app.revertFileModifications()
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
          text: qsTr("Quit")
          onTriggered: confirmedQuit()
        }
      }
    }
  }

  PageStack {
    id: pageStack
    Component.onCompleted: push(mainPage)
    MainPage {
      id: mainPage
      visible: false
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

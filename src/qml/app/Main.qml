/**
 * \file Main.qml
 * Main entry point for QML application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015-2018  Urs Fleisch
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

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQml 2.11
import Kid3 1.1 as Kid3

ApplicationWindow {
  id: root
  visible: true
  visibility: Qt.platform.os === "android" ? "FullScreen" : "Windowed"
  objectName: "mainView"
  title: "Kid3"
  width: constants.gu(100)
  height: constants.gu(100)

  FontLoader {
    id: materialFont
    source: "../icons/kid3material.ttf"
  }

  UiConstants {
    id: constants
  }

  Kid3.FrameEditorObject {
    id: frameEditor
  }

  Kid3.ScriptUtils {
    id: script
  }

  Kid3.ConfigObjects {
    id: configs
  }

  FrameSelectDialog {
    id: frameSelectDialog
    parent: Overlay.overlay

    onFrameSelected: frameEditor.onFrameSelectionFinished(name)

    Connections {
      target: frameEditor
      onFrameSelectionRequested: frameSelectDialog.openFrameNames(frameNames)
    }
  }

  FrameEditDialog {
    id: frameEditDialog
    parent: Overlay.overlay
    onFrameEdited: frameEditor.onFrameEditFinished(frame)

    Connections {
      target: frameEditor
      onFrameEditRequested: frameEditDialog.openFrame(frame)
    }
  }

  MessageDialog {
    property bool doNotRevert: false

    signal completed(bool ok)

    id: saveModifiedDialog
    x: (parent.width - width) / 2
    y: parent.height / 6
    parent: Overlay.overlay
    title: qsTr("Warning")
    text: qsTr("The current folder has been modified.\nDo you want to save it?")
    onYes: {
      saveDirectory(function() {
        completed(true)
      })
    }
    onNo: {
      if (!doNotRevert) {
        app.deselectAllFiles()
        app.revertFileModifications()
      }
      completed(true)
    }
    onCancel: completed(false)

    // Open dialog if any file modified.
    // completed(ok) is signalled with false if canceled.
    function openIfModified() {
      if (app.modified && app.dirName) {
        open()
      } else {
        completed(true)
      }
    }
  }

  MessageDialog {
    property var errorMsgs: []

    signal completed()

    id: writeErrorDialog
    x: (parent.width - width) / 2
    y: parent.height / 6
    parent: Overlay.overlay
    title: qsTr("File Error")
    text: qsTr("Error while writing file:\n") + errorMsgs.join("\n")
    standardButtons: Dialog.Close
    onClosed: {
      completed()
    }
  }

  MessageDialog {
    property string externalFilesDir

    signal completed()

    id: sdCardErrorDialog
    x: (parent.width - width) / 2
    y: parent.height / 6
    parent: Overlay.overlay
    title: qsTr("File Error")
    text: qsTr("SD card is only writable in %1").arg(externalFilesDir)
    standardButtons: Dialog.Close
    onClosed: {
      completed()
    }
  }

  MessageDialog {
    property var errorMsgs: []

    signal completed(bool ok)

    id: changePermissionsDialog
    x: (parent.width - width) / 2
    y: parent.height / 6
    parent: Overlay.overlay
    title: qsTr("File Error")
    text: qsTr("Error while writing file. Do you want to change the permissions?")
          + "\n" + errorMsgs.join("\n")
    standardButtons: Dialog.Yes | Dialog.No
    onYes: {
      completed(true)
    }
    onNo: {
      completed(false)
    }
  }

  Shortcut {
    sequences: ["Esc", "Back"]
    enabled: pageStack.depth > 1
    onActivated: {
      pageStack.pop()
    }
  }

  StackView {
    id: pageStack
    initialItem: mainPage
    anchors.fill: parent

    MainPage {
      id: mainPage
      visible: false
      onConfirmedOpenRequested: confirmedOpenDirectory(path)
      onSaveRequested: saveDirectory(onCompleted)
    }
  }

  Text {
    visible: false
    Component.onCompleted: {
      // Linux Desktop: pixelSize 12 => gu = 8
      // Android Emulator Galaxy Nexus: 32 => gu = 21
      constants.gridUnit = Math.max(8 * font.pixelSize / 12, 8)
      constants.titlePixelSize = 18 * font.pixelSize / 12
      constants.imageScaleFactor = Math.max(font.pixelSize / 12.0, 1.0)
    }
  }

  Component.onCompleted: {
    app.frameEditor = frameEditor
    app.readConfig()
    var path = configs.fileConfig().lastOpenedFile
    if (!path || !script.fileExists(path)) {
      path = script.musicPath()
    }
    app.openDirectory(path)
    if (mainPage.Material) {
      constants.highlightColor = mainPage.Material.primary
      constants.highlightedTextColor = mainPage.Material.foreground
      constants.textColor = mainPage.Material.foreground
      constants.baseColor = mainPage.Material.background
    }
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

  DropArea {
    anchors.fill: parent
    onDropped: {
      if (drop.hasUrls) {
        app.openDropUrls(drop.urls)
      }
    }
  }

  function saveDirectory(onCompleted) {
    var errorFiles = app.saveDirectory()
    var numErrorFiles = errorFiles.length
    if (numErrorFiles > 0) {
      var errorMsgs = [], notWritableFiles = []
      for (var i = 0; i < numErrorFiles; i++) {
        var errorFile = errorFiles[i]
        var slashPos = errorFile.lastIndexOf("/")
        var fileName = slashPos !== -1 ? errorFile.substr(slashPos + 1)
                                       : errorFile
        if (!script.fileIsWritable(errorFile)) {
          errorMsgs.push(qsTr("%1 is not writable").arg(fileName))
          notWritableFiles.push(errorFile)
        } else {
          errorMsgs.push(fileName)
        }
      }
      if (notWritableFiles.length === 0) {
        function resultReceived() {
          writeErrorDialog.completed.disconnect(resultReceived)
          if (onCompleted) {
            onCompleted();
          }
        }
        writeErrorDialog.errorMsgs = errorMsgs
        writeErrorDialog.completed.connect(resultReceived)
        writeErrorDialog.open()
      } else if (Qt.platform.os === "android" &&
                 notWritableFiles[0].substr(0, 19) !== "/storage/emulated/0" &&
                 notWritableFiles[0].substr(0, 9) === "/storage/" &&
                 notWritableFiles[0].indexOf(
                   "Android/data/net.sourceforge.kid3/") === -1) {
        var externalFilesDir = notWritableFiles[0].substr(
              0, notWritableFiles[0].indexOf("/", 9) + 1) +
            "Android/data/net.sourceforge.kid3/"
        if (!script.fileExists(externalFilesDir)) {
          script.makeDir(externalFilesDir)
        }
        function resultReceived() {
          sdCardErrorDialog.completed.disconnect(resultReceived)
          if (onCompleted) {
            onCompleted();
          }
        }
        sdCardErrorDialog.externalFilesDir = externalFilesDir
        sdCardErrorDialog.completed.connect(resultReceived)
        sdCardErrorDialog.open()
      } else {
        function resultReceived(ok) {
          changePermissionsDialog.completed.disconnect(resultReceived)
          if (ok) {
            for (var i = 0; i < notWritableFiles.length; i++) {
              var errorFile = notWritableFiles[i]
              var perms = script.getFilePermissions(errorFile)
              script.setFilePermissions(errorFile, perms | 0x0200)
            }
            // Try again
            app.saveDirectory()
          }
          if (onCompleted) {
            onCompleted();
          }
        }
        changePermissionsDialog.errorMsgs = errorMsgs
        changePermissionsDialog.completed.connect(resultReceived)
        changePermissionsDialog.open()
      }
    } else {
      if (onCompleted) {
        onCompleted();
      }
    }
  }

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

  onClosing: {
    confirmedQuit()
    if (app.modified) {
      close.accepted = false
    }
  }
}

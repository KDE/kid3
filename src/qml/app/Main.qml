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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQml 2.2
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

    onFrameSelected: frameEditor.onFrameSelectionFinished(name)

    Connections {
      target: frameEditor
      onFrameSelectionRequested: frameSelectDialog.openFrameNames(frameNames)
    }
  }

  FrameEditDialog {
    id: frameEditDialog
    parent: ApplicationWindow.overlay
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
    x: (root.width - width) / 2
    y: root.height / 6
    parent: ApplicationWindow.overlay
    title: qsTr("Warning")
    text: qsTr("The current directory has been modified.\nDo you want to save it?")
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
    app.openDirectory(configs.fileConfig().lastOpenedFile)
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

/**
 * \file FileSelectDialog.qml
 * File select dialog.
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Qt.labs.folderlistmodel 2.1

Dialog {
  id: page

  signal finished(string path)
  signal fileSelected(string fileName)
  property bool showDotAndDotDot: true
  property bool showHidden: true
  property bool showDirsFirst: true
  property bool saveMode: false
  property string currentFile: ""
  property string folder: ""
  property var nameFilters: ["*.*"]

  title: qsTr("Open")
  modal: true
  x: (root.width - width) / 2
  y: (root.height - height) / 2
  width: Math.min(root.width, constants.gu(65))
  height: Math.min(root.height, constants.gu(80))
  standardButtons: Dialog.Ok | Dialog.Cancel
  onAccepted: page.finished(getCurrentFilePath())
  onRejected: page.finished(null)

  function getCurrentFilePath() {
    return folderField.text + "/" + currentFileField.text
  }

  function simplifyPath(path) {
    if (typeof path === "object") {
      path = path.toString()
    }
    if (path.substr(0,7) === "file://") {
      path = path.substr(7)
    }
    return path
  }

  function setFolder(path) {
    path = simplifyPath(path)
    for (;;) {
      if (script.classifyFile(path) === "/") {
        break;
      }
      var slashPos = path.lastIndexOf("/")
      if (slashPos === -1) {
        path = "/";
        break;
      } else {
        path = path.substring(0, slashPos)
      }
    }
    folderListModel.folder = "file://" + path
  }

  function setCurrentFile(name) {
    var idx = folderListModel.indexOf(folderListModel.folder + "/" + name)
    if (idx >= 0) {
      fileListView.currentIndex = idx
    }
  }

  ColumnLayout {
    anchors.fill: parent
    GridLayout {
      id: pathLayout
      columns: 2
      Label {
        text: qsTr("Folder")
      }
      TextField {
        id: folderField
        Layout.fillWidth: true
        selectByMouse: true
        onEditingFinished: {
          setFolder(text)
        }
      }
      Label {
        text: qsTr("File")
      }
      TextField {
        id: currentFileField
        Layout.fillWidth: true
        selectByMouse: true
        onEditingFinished: {
          setCurrentFile(text)
        }
      }
    }
    ListView {
      id: fileListView
      width: parent.width
      anchors.top: pathLayout.bottom
      anchors.topMargin: constants.margins
      anchors.bottom: parent.bottom

      clip: true

      model: FolderListModel {
        id: folderListModel
        showDotAndDotDot: page.showDotAndDotDot
        showHidden: page.showHidden
        showDirsFirst: page.showDirsFirst
        showOnlyReadable: true
        folder: page.folder
        nameFilters: page.nameFilters
      }

      delegate: Standard {
        id: fileDelegate
        progression: fileIsDir
        onClicked: {
          if (!fileIsDir) {
            ListView.view.currentIndex = index
            currentFileField.text = fileName
            return;
          }
          if (!page.saveMode) {
            currentFileField.text = ""
          }
          ListView.view.currentIndex = -1
          var currentPath = simplifyPath(folderField.text)
          if (currentPath === "") {
            currentPath = "/"
          }
          var selectedDirName = fileName
          if (selectedDirName === "..") {
            if (currentPath !== "/") {
              folderField.text = simplifyPath(folderListModel.parentFolder)
              folderListModel.folder = folderListModel.parentFolder
            }
          } else if (selectedDirName === ".") {
            folderField.text = simplifyPath(folderListModel.folder)
          } else {
            if (currentPath === "/") {
              folderListModel.folder = "file:///" + selectedDirName
            } else {
              folderListModel.folder += "/" + selectedDirName
            }
            if (currentPath[currentPath.length - 1] !== "/") {
              currentPath += "/"
            }
            currentPath += selectedDirName
            folderField.text = currentPath
          }
        }
        highlighted: ListView.isCurrentItem
        background: Rectangle {
          color: highlighted ? constants.highlightColor : "transparent"
        }

        Row {
          anchors.fill: parent

          Label {
            id: fileText
            anchors.verticalCenter: parent.verticalCenter
            text: fileName
            color: fileDelegate.highlighted
              ? constants.highlightedTextColor : constants.textColor
          }
        }
      }
    }
  }
  onOpened: {
    folderField.text = page.folder
    currentFileField.text = page.currentFile
    setFolder(page.folder)
    setCurrentFile(page.currentFile)
  }
}

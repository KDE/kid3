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

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
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
  x: (parent.width - width) / 2
  y: (parent.height - height) / 2
  width: Math.min(parent.width, constants.gu(65))
  height: Math.min(parent.height, constants.gu(80))
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
    if (path.substr(0, 7) === "file://") {
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
      Layout.maximumWidth: parent.width
      columns: 3
      Label {
        id: folderLabel
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
      IconButton {
        id: menuButton
        iconName: "navigation-menu"
        color: folderLabel.color
        onClicked: menu.open()
        Menu {
          id: menu

          // https://stackoverflow.com/questions/49599322/qt-getting-cleaner-storage-volumes-info-on-android
          function getMountedVolumes() {
            var vols = script.mountedVolumes()
            var result = []
            var i
            switch (Qt.platform.os) {
            case "android":
              var sd = /\/storage\/[0-9A-F]{4}-[0-9A-F]{4}/, sdc = 1
              var usb = /\/mnt\/media_rw\/[0-9A-F]{4}-[0-9A-F]{4}/, usbc = 1
              result.push({
                            name: "Internal Storage",
                            rootPath: "/storage/emulated/0",
                            isValid: true,
                            isReady: true
                          })
              for (i = 0; i < vols.length; ++i) {
                if (sd.test(vols[i].rootPath)) {
                  vols[i].name = "SD Card " + sdc++
                  result.push(vols[i])
                } else if (usb.test(vols[i].rootPath)) {
                  vols[i].name = "USB drive " + usbc++
                  result.push(vols[i])
                }
              }
              return result
            case "linux":
              for (i = 0; i < vols.length; ++i) {
                var path = vols[i].rootPath
                if (path.indexOf("/media/") !== -1 ||
                    (path.substr(0, 4) !== "/run" &&
                     path.substr(0, 5) !== "/snap")) {
                  result.push(vols[i])
                }
              }
              return result
            }
            return vols
          }

          function addEntry(text, path) {
            // This will cause a warning, is there a workaround?
            // Created graphical object was not placed in the graphics scene.
            menu.addItem(menuItem.createObject(menu, {text: text, path: path}))
          }

          function setupEntries() {
            while (menu.itemAt(0)) {
              menu.removeItem(menu.itemAt(0))
            }
            menu.addEntry("Music", script.musicPath())
            var vols = getMountedVolumes();
            for (var i = 0; i < vols.length; ++i) {
              var vol = vols[i]
              if (vol.isValid && vol.isReady && !vol.isReadOnly) {
                menu.addEntry(vol.name || vol.displayName || vol.path,
                              vol.rootPath)
              }
            }
          }

          Component {
            id: menuItem
            MenuItem {
              property string path
              onTriggered: {
                folderField.text = path
                setFolder(path)
                if (!page.saveMode) {
                  currentFileField.text = ""
                }
              }
            }
          }
        }
      }
      Label {
        text: qsTr("File")
      }
      TextField {
        id: currentFileField
        Layout.fillWidth: true
        Layout.columnSpan: 2
        selectByMouse: true
        onEditingFinished: {
          setCurrentFile(text)
        }
      }
    }
    ListView {
      id: fileListView
      Layout.maximumWidth: parent.width
      Layout.fillWidth: true
      Layout.fillHeight: true

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
    menu.setupEntries()
  }
}

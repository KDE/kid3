/**
 * \file MainPage.qml
 * Main page.
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

Page {
  id: page

  function updateCurrentSelection() {
    collapsibleV1.acceptEdit()
    collapsibleV2.acceptEdit()
    collapsibleV3.acceptEdit()
    app.frameModelsToTags()
    if (app.selectionInfo.singleFileSelected) {
      app.selectionInfo.fileName = collapsibleFile.fileName
    }
  }

  function currentFilePath() {
    return fileList.currentFilePath()
  }

  title: (app.dirName.split("/").pop() || "/") +
         (app.modified ? qsTr(" [modified]") : "") +
         (app.filtered ? qsTr(" [filtered %1/%2]")
             .arg(app.filterPassedCount).arg(app.filterTotalCount) : "") +
         " - Kid3"

  Shortcut {
    sequence: "Menu"
    onActivated: menu.open()
  }

  header: ToolBar {
    height: constants.controlHeight
    IconButton {
      id: prevButton
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      iconName: page.StackView.view.depth > 1 ? "go-previous"
                                              : "drawer";
      width: height
      onClicked: {
        if (page.StackView.view.depth > 1) {
          page.StackView.view.pop()
        } else if (drawer.position === 1.0) {
          drawer.close()
        } else {
          drawer.open()
        }
      }
    }
    Label {
      id: titleLabel
      anchors.left: prevButton.right
      anchors.right: buttonRow.left
      anchors.verticalCenter: parent.verticalCenter
      clip: true
      text: page.title
      MouseArea {
        anchors.fill: parent
        onPressAndHold: {
          openDialog.filePath = app.dirName
          openDialog.open()
        }

        FileSelectDialog {
          id: openDialog
          parent: ApplicationWindow.overlay
          title: qsTr("Open")
          onFinished: if (path) root.confirmedOpenDirectory(path)
        }
      }
    }
    Row {
      id: buttonRow
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      spacing: constants.spacing
      IconButton {
        iconName: "go-up"
        width: height
        onClicked: {
          var parentDir = fileList.parentFilePath()
          if (parentDir) {
            confirmedOpenDirectory(parentDir)
          }
        }
      }
      IconButton {
        property bool selectAll: true
        iconName: "select"
        width: height
        onClicked: {
          if (selectAll) {
            app.selectAllFiles()
          } else {
            app.deselectAllFiles()
          }
          selectAll = !selectAll
        }
      }
      IconButton {
        id: previousButton
        iconName: "go-previous"
        visible: body.state != "narrow" || drawer.position === 0.0
        width: height
        onClicked: app.previousFile()
      }
      IconButton {
        iconName: "go-next"
        visible: previousButton.visible
        width: height
        onClicked: app.nextFile()
      }
      IconButton {
        id: menuButton
        iconName: "navigation-menu"
        width: height
        onClicked: menu.open()
        Menu {
          id: menu
          width: constants.gu(35)
          MenuItem {
            text: qsTr("About")
            onTriggered: aboutDialog.open()

            AboutDialog {
              id: aboutDialog
              parent: ApplicationWindow.overlay
            }
          }
          MenuItem {
            text: qsTr("Open")
            onTriggered: {
              openDialog.filePath = app.dirName
              openDialog.open()
            }
          }
          MenuItem {
            text: qsTr("Save")
            onTriggered: {
              var errorFiles = app.saveDirectory()
              if (errorFiles.length > 0) {
                console.debug("Save error:" + errorFiles)
              }
            }
          }
          MenuItem {
            text: qsTr("Settings")
            onTriggered: page.StackView.view.push(settingsPage)

            SettingsPage {
              id: settingsPage
              visible: false
            }
          }
          MenuItem {
            text: qsTr("Automatic Import")
            onTriggered: page.StackView.view.push(batchImportPage)

            BatchImportPage {
              id: batchImportPage
              visible: false
            }
          }
          MenuItem {
            text: qsTr("Create Playlist")
            onTriggered: app.writePlaylist()
          }
          MenuItem {
            text: qsTr("Rename Directory")
            onTriggered: page.StackView.view.push(renameDirPage)

            RenameDirectoryPage {
              id: renameDirPage
              visible: false
            }
          }
          MenuItem {
            text: qsTr("Number Tracks")
            onTriggered: numberTracksDialog.open()

            NumberTracksDialog {
              id: numberTracksDialog
              parent: ApplicationWindow.overlay
            }
          }
          MenuItem {
            text: qsTr("Filter")
            onTriggered: page.StackView.view.push(filterPage)

            FilterPage {
              id: filterPage
              visible: false
            }
          }
          MenuItem {
            text: qsTr("Apply Filename Format")
            onTriggered: app.applyFilenameFormat()
          }
          MenuItem {
            text: qsTr("Apply Tag Format")
            onTriggered: app.applyTagFormat()
          }
          MenuItem {
            text: qsTr("Apply Text Encoding")
            onTriggered: app.applyTextEncoding()
          }
          MenuItem {
            text: qsTr("Convert ID3v2.3 to ID3v2.4")
            onTriggered: app.convertToId3v24()
          }
          MenuItem {
            text: qsTr("Convert ID3v2.4 to ID3v2.3")
            onTriggered: app.convertToId3v23()
          }
          MenuItem {
            text: qsTr("Revert")
            onTriggered: app.revertFileModifications()
          }
          MenuItem {
            text: qsTr("Quit")
            onTriggered: confirmedQuit()
          }

        }
      }
    }
  }
  Item {
    id: body

    property int rightSideSpace: width - drawer.width

    anchors.fill: parent

    Drawer {
      id: drawer
      y: header.height
      width: Math.min(constants.gu(44), body.width - constants.gu(4))
      height: parent.height - y
      modal: false
      interactive: false

      FileList {
        id: fileList
        color: constants.baseColor
        anchors.fill: parent

        onFileActivated: {
          if (body.state === "narrow") {
            drawer.close()
          }
        }
      }
    }

    Rectangle {
      id: rightSide

      color: constants.baseColor
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      width: body.width - drawer.width * drawer.position

      Flickable {
        id: rightSideFlickable
        anchors.fill: parent
        contentWidth: width
        contentHeight: collapsibleColumn.height
        clip: true
        Column {
          id: collapsibleColumn
          width: parent.width

          FileCollapsible {
            id: collapsibleFile
            anchors.topMargin: constants.margins
            anchors.left: parent.left
            anchors.right: parent.right
          }

          TagCollapsible {
            id: collapsibleV1
            tagNr: 0
            anchors.topMargin: constants.margins
            anchors.left: parent.left
            anchors.right: parent.right
          }

          TagCollapsible {
            id: collapsibleV2
            tagNr: 1
            anchors.topMargin: constants.margins
            anchors.left: parent.left
            anchors.right: parent.right
          }

          TagCollapsible {
            id: collapsibleV3
            tagNr: 2
            anchors.topMargin: constants.margins
            anchors.left: parent.left
            anchors.right: parent.right
          }

          PictureCollapsible {
            id: collapsiblePicture
            anchors.topMargin: constants.margins
            anchors.left: parent.left
            anchors.right: parent.right
          }
        }
      }
    }

    states: [
      State {
        name: ""
        StateChangeScript {
          script: drawer.open()
        }
      },
      State {
        name: "hidden"
        when: page.StackView.view.depth > 1
        StateChangeScript {
          script: drawer.close()
        }
      },
      State {
        name: "narrow"
        when: body.rightSideSpace < constants.gu(50)
        PropertyChanges {
          target: drawer
          interactive: page.StackView.view.depth === 1
        }
        PropertyChanges {
          target: rightSide
          width: body.width
        }
      }
    ]
  }
}

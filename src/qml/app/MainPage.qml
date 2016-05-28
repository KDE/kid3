/**
 * \file MainPage.qml
 * Main page.
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

Page {
  id: page

  property Item _buttons: Row {
    spacing: constants.spacing
    Button {
      iconName: "go-up"
      width: height
      onClicked: {
        var parentDir = fileList.parentFilePath()
        if (parentDir) {
          confirmedOpenDirectory(parentDir)
        }
      }
    }
    Button {
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
    Button {
      id: previousButton
      iconName: "go-previous"
      visible: body.state != "narrow" || rightSide.raised
      width: height
      onClicked: app.previousFile()
    }
    Button {
      iconName: "go-next"
      visible: previousButton.visible
      width: height
      onClicked: app.nextFile()
    }
    Button {                                                         //@!Ubuntu
      id: menuButton                                                 //@!Ubuntu
      iconName: "navigation-menu"                                    //@!Ubuntu
      width: height                                                  //@!Ubuntu
      onClicked: constants.openPopup(mainMenuPopoverComponent, menuButton) //@!Ubuntu
    }                                                                //@!Ubuntu
  }

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
         (app.filtered ? qsTr(" [filtered]") : "") +
         " - Kid3"
  flickable: rightSideFlickable //@!Ubuntu
  actionButtons: _buttons //@!Ubuntu
  Item {
    id: body

    property int rightSideSpace: width - fileList.width - constants.margins

    anchors.fill: parent
    FileList {
      id: fileList

      raised: true
      onClicked: {
        if (body.state == "narrow")
          rightSide.raised = false
        raised = true
      }

      color: constants.backgroundColor
      anchors.left: parent.left
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      width: Math.min(constants.gu(44), body.width - constants.gu(4))
      //actionButtons: _buttons  //@Ubuntu
    }

    RaisableRectangle {
      id: rightSide

      raised: true
      onClicked: {
        if (body.state == "narrow")
          fileList.raised = false
        raised = true
      }

      color: constants.backgroundColor
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      width: parent.rightSideSpace

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
            //onMainMenuRequested:  constants.openPopup(mainMenuPopoverComponent, caller) //@Ubuntu
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
        name: "narrow"
        when: body.rightSideSpace < constants.gu(50)
        PropertyChanges {
          target: fileList
        }
        PropertyChanges {
          target: rightSide
          raised: !fileList.raised
          width: Math.min(constants.gu(50), body.width - constants.gu(4))
        }
      }
    ]
  }
}

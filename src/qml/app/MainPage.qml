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

  function updateCurrentSelection() {
    collapsibleV1.acceptEdit()
    collapsibleV2.acceptEdit()
    app.frameModelsToTags()
    if (app.selectionInfo.singleFileSelected) {
      app.selectionInfo.fileName = collapsibleFile.fileName
    }
  }

  function currentFilePath() {
    return fileList.currentFilePath()
  }

  title: app.dirName.split("/").pop() +
         (app.modified ? qsTr(" [modified]") : "") +
         (app.filtered ? qsTr(" [filtered]") : "") +
         " - Kid3"

  Item {
    id: body

    property int rightSideSpace: width - fileList.width - 3 * constants.margins

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
      anchors.margins: constants.margins
      width: constants.gu(44)
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
      anchors.margins: constants.margins
      width: parent.rightSideSpace

      Flickable {
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
            onMainMenuRequested:  constants.openPopup(mainMenuPopoverComponent,
                                                      caller)
          }

          Tag1Collapsible {
            id: collapsibleV1
            anchors.topMargin: constants.margins
            anchors.left: parent.left
            anchors.right: parent.right
          }

          Tag2Collapsible {
            id: collapsibleV2
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
          raised: !rightSide.raised
        }
        PropertyChanges {
          target: rightSide
          width: constants.gu(50)
        }
      }
    ]
  }
}

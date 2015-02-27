/**
 * \file FileCollapsible.qml
 * Collapsible with file information.
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
import "ComponentsQtQuick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3 1.0

Collapsible {
  id: fileCollapsible

  property alias fileName: fileNameEdit.text

  signal mainMenuRequested(variant caller)

  text: qsTr("File") + ": " + app.selectionInfo.detailInfo
  checked: true
  buttons: [
    Button {
      id: mainMenuButton
      iconName: "navigation-menu"
      width: height
      onClicked: fileCollapsible.mainMenuRequested(mainMenuButton)
    }
  ]

  content: Item {
    width: parent.width
    height: fileNameEdit.height + constants.gu(2)
    Image {
      id: fileNameModifiedImage
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      width: 16
      height: 16
      source: "image://kid3/fileicon/" +
              ( app.selectionInfo.fileNameChanged ? "modified" : "null")
    }
    Label {
      id: fileNameLabel
      anchors.left: fileNameModifiedImage.right
      anchors.verticalCenter: parent.verticalCenter
      text: "Name:"
    }
    TextField {
      id: fileNameEdit
      anchors.left: fileNameLabel.right
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
      text: app.selectionInfo.fileName
    }
  }
}

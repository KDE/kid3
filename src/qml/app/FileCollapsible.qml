/**
 * \file FileCollapsible.qml
 * Collapsible with file information.
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

Collapsible {
  id: fileCollapsible

  property alias fileName: fileNameEdit.text

  text: qsTr("File") + ": " + app.selectionInfo.detailInfo
  checked: true

  content: Item {
    width: parent.width
    height: fileNameEdit.height + constants.gu(2)
    Item {
      id: fileNameModifiedImage
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      width: constants.gu(2)
      height: constants.gu(2)
      Text {
        font.family: materialFont.name
        font.pixelSize: 16
        text: "M"
        color: fileNameLabel.color
        visible: app.selectionInfo.fileNameChanged
      }
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
      selectByMouse: true
      focus: true
      onAccepted: {
        focus = false
      }
      onActiveFocusChanged: {
        if (!activeFocus) {
          app.selectionInfo.fileName = text
        }
      }
    }
  }
}

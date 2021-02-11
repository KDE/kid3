/**
 * \file Standard.qml
 * Standard list item.
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

ItemDelegate {
  id: listItem
  property bool progression
  property alias control: controlContainer.control

  width: parent ? parent.width : constants.gu(31)
  height: constants.rowHeight

  contentItem: Item {
    anchors.fill: parent

    Text {
      id: textLabel
      anchors.left: parent.left
      anchors.right: controlContainer.left
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
      color: highlighted
             ? constants.highlightedTextColor :constants.textColor
      rightPadding: listItem.spacing
      text: listItem.text
      font: listItem.font
      elide: Text.ElideRight
      visible: listItem.text
      horizontalAlignment: Text.AlignLeft
      verticalAlignment: Text.AlignVCenter
    }
    Item {
      id: controlContainer
      property Item control
      width: control ? control.width : undefined
      height: control ? control.height : undefined
      anchors.right: progression ? progressionImage.left : parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
      onControlChanged: {
        if (control) control.parent = controlContainer;
      }
    }
    Text {
      id: progressionImage
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
      font.family: materialFont.name
      font.pixelSize: 24
      text: ">"
      color: highlighted
             ? constants.highlightedTextColor :constants.textColor
      visible: progression
    }
  }
}

/**
 * \file Standard.qml
 * Standard list item.
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

Empty {
  id: listItem
  property alias text: textLabel.text
  property bool progression
  property alias control: controlContainer.control

  __acceptEvents: false

  Text {
    id: textLabel
    anchors.left: parent.left
    anchors.right: controlContainer.left
    anchors.verticalCenter: parent.verticalCenter
    anchors.margins: constants.margins
    color: selected
           ? constants.palette.highlightedText :constants.palette.text
  }
  Item {
    id: controlContainer
    property Item control
    width: control ? control.width : undefined
    height: control ? control.height : undefined
    anchors.right: progression ? progressionLabel.left : parent.right
    anchors.verticalCenter: parent.verticalCenter
    anchors.margins: constants.margins
    onControlChanged: {
      if (control) control.parent = controlContainer;
    }
    Connections {
      target: listItem.__mouseArea

      onClicked: {
        if (control && listItem.__mouseArea.mouseX < progressionLabel.x) {
          if (control.enabled && control.hasOwnProperty("clicked"))
            control.clicked();
        } else {
          listItem.clicked();
        }
      }
    }
  }

  Text {
    id: progressionLabel
    anchors.right: parent.right
    anchors.verticalCenter: parent.verticalCenter
    anchors.margins: constants.margins
    text : ">"
    visible: progression
  }
}

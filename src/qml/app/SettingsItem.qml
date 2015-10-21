/**
 * \file Standard.qml
 * Standard list item.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Oct 2015
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
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu

Rectangle {
  id: listItem

  property bool selected: false
  property alias text: textLabel.text
  property alias control: controlContainer.control

  property bool horizontal : !control ||
                 control.width + textLabel.width + 2 * constants.margins < width

  signal clicked()

  width: parent ? parent.width : constants.gu(31)
  height: horizontal ? constants.rowHeight : 2 * constants.rowHeight
  color: selected
         ? constants.palette.highlight : constants.palette.window //@!Ubuntu
         //? Theme.palette.selected.background : Theme.palette.normal.background //@Ubuntu

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    onClicked: {
      if (control && mouseX >= control.x) {
        if (control.enabled && control.hasOwnProperty("clicked"))
          control.clicked();
      } else {
        listItem.clicked();
      }
    }
  }

  Text {
    id: textLabel
    height: constants.rowHeight
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.margins: constants.margins
    anchors.topMargin: 2 * constants.margins //@Ubuntu
    color: selected
           ? constants.palette.highlightedText :constants.palette.text //@!Ubuntu
           //? Theme.palette.selected.fieldText : Theme.palette.normal.fieldText //@Ubuntu
  }
  Item {
    id: controlContainer
    property Item control
    width: horizontal ? parent.width - 2 * constants.margins - textLabel.width
                      : parent.width - 1 * constants.margins
    height: constants.rowHeight
    anchors.right: parent.right
    anchors.bottom: divider.top
    onControlChanged: {
      if (control) {
        control.parent = controlContainer
        control.anchors.right = controlContainer.right
        control.anchors.verticalCenter = controlContainer.verticalCenter
        control.anchors.margins = constants.margins
      }
    }
  }
  ThinDivider {
    id: divider
    anchors.bottom: parent.bottom
  }
}

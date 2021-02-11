/**
 * \file Empty.qml
 * Empty list view base component.
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

Rectangle {
  id: emptyListItem

  property bool selected: false
  property bool __acceptEvents: true
  property alias __mouseArea: mouseArea

  signal clicked()

  width: parent ? parent.width : constants.gu(31)
  height: constants.rowHeight
  color: selected
         ? constants.highlightColor : "transparent"

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    onClicked: {
      if (emptyListItem.__acceptEvents) {
        emptyListItem.clicked()
      }
    }
  }
  ThinDivider {
    id: divider
    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
  }
}

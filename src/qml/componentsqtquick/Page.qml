/**
 * \file Page.qml
 * Page managed by page stack.
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

Rectangle {
  property alias title: titleLabel.text
  default property alias contents: contentsItem.data
  property bool active: visible

  anchors.fill: parent
  Row {
    id: titleRow
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.margins: constants.margins
    height: constants.rowHeight
    Button {
      border.width: 0
      iconName: "go-previous"
      visible: pageStack.canPop
      onClicked: pageStack.pop()
    }
    Text {
      id: titleLabel
      anchors.verticalCenter: parent.verticalCenter
    }
  }
  Item {
    id: contentsItem
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: titleRow.bottom
    anchors.bottom: parent.bottom
    anchors.margins: constants.margins
  }
}

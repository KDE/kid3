/**
 * \file CheckBox.qml
 * Check box.
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
  id: checkfield

  property bool checked

  signal clicked

  width: constants.rowHeight
  height: constants.rowHeight
  color: constants.palette.base

  Rectangle {
    anchors.centerIn: parent
    width: parent.width - 4
    height: parent.height - 4
    border.width: 1
    border.color: "black"
  }

  Text {
    id: checkboxText
    text: checkfield.checked ? "x" : ""
    anchors.centerIn: parent
  }

  MouseArea {
    anchors.fill: parent
    onClicked: {
      checkfield.checked = !checkfield.checked
      checkfield.clicked()
    }
  }
}

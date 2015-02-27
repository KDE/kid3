/**
 * \file ComboBox.qml
 * Combo box.
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

Item {
  id: comboBox
  property alias model: dropDown.model
  // Can be set to make sure that the dropDown is on top
  property alias dropDownParent: dropDown.dropDownRoot
  property alias currentText: selectedItemText.text
  property alias currentIndex: dropDown.currentIndex

  implicitWidth: constants.gu(25)
  implicitHeight: constants.gu(4)

  Rectangle {
    id: selectedItem
    radius: 4
    width: parent.width
    height: parent.height
    anchors.verticalCenter: parent.verticalCenter
    color: constants.comboBoxColor
    smooth: true
    Text {
      id: selectedItemText
      anchors.left: parent.left
      anchors.margins: constants.margins
      anchors.verticalCenter: parent.verticalCenter
      // Check to avoid error if model is not an array
      text: !!comboBox.model && !!comboBox.model.length
            ? model[comboBox.currentIndex] :
              model.hasOwnProperty("get")
              ? model.get(comboBox.currentIndex).display
              : ""

      smooth: true
    }

    MouseArea {
      anchors.fill: parent;
      onClicked: dropDown.toggleVisible()
    }
  }

  DropDownList {
    id: dropDown
    width: comboBox.width
    anchors.top: selectedItem.bottom
    anchors.margins: 2

    onCurrentTextChanged: selectedItemText.text = currentText
  }
}

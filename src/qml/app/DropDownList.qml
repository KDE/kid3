/**
 * \file DropDownList.qml
 * Drop down list.
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
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu

Item {
  id: dropDown

  property Item dropDownRoot
  property variant model
  property string currentText
  property alias currentIndex: listView.currentIndex
  property alias delegate: listView.delegate

  signal clicked

  height: 0
  z: 0
  clip: true

  function show() {
    state = "dropDown"
  }

  function hide() {
    state = ""
  }

  function setVisible(enable) {
    state = enable ? "dropDown" : ""
  }

  function toggleVisible() {
    state = state === "dropDown" ? "": "dropDown"
  }

  function calculateHeight() {
    var parentHeight = dropDownRoot.height
    // @todo Find a reliable way to get the row height.
    var listViewHeight = listView.count * constants.gu(6)
    return listViewHeight <= parentHeight
        ? listViewHeight : parentHeight
  }

  function calculateY(dropDownHeight) {
    var yPos = dropDown.mapToItem(dropDownRoot, 0, 0).y
    var yOffset = dropDownRoot.height - yPos - dropDownHeight
    if (yOffset < 0)
      yPos += yOffset
    if (yPos < 0)
      yPos = 0
    return yPos
  }

  ListView {
    id: listView
    anchors.fill: parent
    model: dropDown.model
    Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Beginning)

    delegate: Rectangle {
      width: parent.width
      height: delegateText.height
      Standard {
        id: delegateText
        anchors.fill: parent
        text: model.modelData || model.display
        selected: listView.currentIndex == index

        onClicked: {
          dropDown.state = ""
          currentText = text
          listView.currentIndex = index;
          dropDown.clicked()
        }
      }
    }
  }

  MouseArea {
    id: rootMouseArea
    parent: dropDownRoot
    anchors.fill: parent
    z: dropDown.z - 1
    onClicked: dropDown.state = ""
  }

  states: State {
    name: "dropDown";
    ParentChange {
      target: dropDown
      parent: dropDown.dropDownRoot
      height: dropDown.calculateHeight()
      y: dropDown.calculateY(height)
    }
    PropertyChanges {
      target: dropDown
      z: 100
    }
  }
}

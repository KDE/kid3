/**
 * \file StringListEditPage.qml
 * Page to edit a list of strings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Feb 2019
 *
 * Copyright (C) 2019  Urs Fleisch
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

Page {
  id: page

  readonly property string mapSeparator: String.fromCharCode(0x00a0) +
                       String.fromCharCode(0x2192) + String.fromCharCode(0x00a0)
  property bool hasMap: false

  function setList(lst) {
    hasMap = false
    listView.model.clear()
    for (var i = 0; i < lst.length; i++) {
      listView.model.append({"name": lst[i]})
    }
  }

  function getList() {
    var lst = []
    for (var i = 0; i < listView.model.count; i++) {
      lst.push(listView.model.get(i).name)
    }
    return lst
  }

  function setMap(map) {
    hasMap = true
    listView.model.clear()
    for (var key in map) {
      if (map.hasOwnProperty(key)) {
        listView.model.append({"name": key + mapSeparator + map[key]})
      }
    }
  }

  function getMap() {
    var map = {}
    for (var i = 0; i < listView.model.count; i++) {
      var s = listView.model.get(i).name
      var sepPos = s.indexOf(mapSeparator)
      if (sepPos !== -1) {
        map[s.substring(0, sepPos)] = s.substring(sepPos + mapSeparator.length)
      }
    }
    return map
  }

  function setCurrentIndex(index) {
    listView.currentIndex = index
  }

  function getCurrentIndex() {
    return listView.currentIndex
  }

  title: qsTr("Edit")

  Dialog {
    id: editDialog

    property alias text: textLineEdit.text

    signal completed(bool ok)

    modal: true
    width: Math.min(root.width, constants.gu(70))
    x: (root.width - width) / 2
    y: 0
    standardButtons: Dialog.Ok | Dialog.Cancel

    TextField {
      id: textLineEdit
      width: parent.width
    }

    onAccepted: completed(!hasMap || text.indexOf(mapSeparator) !== -1)
    onRejected: completed(false)
  }

  header: ToolBar {
    IconButton {
      id: prevButton
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      iconName: "go-previous"
      color: titleLabel.color
      width: visible ? height : 0
      visible: page.StackView.view && page.StackView.view.depth > 1
      onClicked: page.StackView.view.pop()
    }
    Label {
      id: titleLabel
      anchors.left: prevButton.right
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      clip: true
      text: page.title
    }
  }

  RowLayout {
    anchors {
      fill: parent
      margins: constants.margins
    }
    ListView {
      id: listView
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
      model: ListModel {}
      delegate: Standard {
        text: name
        highlighted: ListView.view.currentIndex === index
        onClicked: ListView.view.currentIndex = index
        background: Rectangle {
          color: highlighted ? constants.highlightColor : "transparent"
        }
      }
    }
    ColumnLayout {
      Layout.alignment: Qt.AlignTop
      Label {
        id: invisibleLabel
        visible: false
      }
      IconButton {
        iconName: "add"
        color: invisibleLabel.color
        onClicked: {
          function modifyIfCompleted(ok) {
            editDialog.completed.disconnect(modifyIfCompleted)
            if (ok) {
              listView.model.append({"name": editDialog.text})
            }
          }

          editDialog.text = hasMap ? mapSeparator : ""
          editDialog.completed.connect(modifyIfCompleted)
          editDialog.open()
        }
      }
      IconButton {
        iconName: "go-up"
        color: invisibleLabel.color
        onClicked: {
          var idx = listView.currentIndex
          if (idx > 0) {
            listView.model.move(idx, idx - 1, 1)
            listView.currentIndex = idx - 1
          }
        }
      }
      IconButton {
        iconName: "go-down"
        color: invisibleLabel.color
        onClicked: {
          var idx = listView.currentIndex
          if (idx >= 0 && idx < listView.model.count - 1) {
            listView.model.move(idx, idx + 1, 1)
            listView.currentIndex = idx + 1
          }
        }
      }
      IconButton {
        iconName: "edit"
        color: invisibleLabel.color
        onClicked: {
          var idx = listView.currentIndex
          if (idx >= 0) {
            function modifyIfCompleted(ok) {
              editDialog.completed.disconnect(modifyIfCompleted)
              if (ok) {
                listView.model.set(idx, {"name": editDialog.text})
              }
            }

            editDialog.text = listView.model.get(idx).name
            editDialog.completed.connect(modifyIfCompleted)
            editDialog.open()
          }
        }
      }
      IconButton {
        iconName: "remove"
        color: invisibleLabel.color
        onClicked: {
          var idx = listView.currentIndex
          if (idx >= 0) {
            listView.model.remove(idx, 1)
          }
        }
      }
    }
  }
}

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

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4

Page {
  id: page

  property alias model: listView.model
  property alias currentIndex: listView.currentIndex
  property alias count: listView.count
  property Dialog editDialog: textEditDialog

  property var onAddClicked: function() {
    function modifyIfCompleted(ok) {
      editDialog.completed.disconnect(modifyIfCompleted)
      if (ok) {
        addElement(editDialog.getElement())
      }
    }

    editDialog.setElement({ name: "" })
    editDialog.completed.connect(modifyIfCompleted)
    editDialog.open()
  }

  property var onEditClicked: function() {
    var idx = listView.currentIndex
    if (idx >= 0) {
      function modifyIfCompleted(ok) {
        editDialog.completed.disconnect(modifyIfCompleted)
        if (ok) {
          listView.model.set(idx, editDialog.getElement())
        }
      }

      editDialog.setElement(listView.model.get(idx))
      editDialog.completed.connect(modifyIfCompleted)
      editDialog.open()
    }
  }

  function addElement(element) {
    model.append(element)
    currentIndex = count - 1
  }

  function setElements(lst) {
    listView.model.clear()
    for (var i = 0; i < lst.length; i++) {
      listView.model.append({ "name": lst[i] })
    }
  }

  function getElements() {
    var lst = []
    for (var i = 0; i < listView.model.count; i++) {
      lst.push(listView.model.get(i).name)
    }
    return lst
  }

  title: qsTr("Edit")

  Dialog {
    id: textEditDialog

    signal completed(bool ok)

    function setElement(element) {
      textLineEdit.text = element.name
    }

    function getElement() {
      return { name: textLineEdit.text }
    }

    modal: true
    width: Math.min(parent.width, constants.gu(70))
    x: (parent.width - width) / 2
    y: 0
    standardButtons: Dialog.Ok | Dialog.Cancel

    TextField {
      id: textLineEdit
      width: parent.width
      selectByMouse: true
    }

    onAccepted: completed(true)
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
        onClicked: onAddClicked()
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
        onClicked: onEditClicked()
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

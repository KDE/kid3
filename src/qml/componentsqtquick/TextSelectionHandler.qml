/**
 * \file TextSelectionHandler.qml
 * Handler with edit menu for text selection in TextArea and TextField.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Nov 2015
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

MouseArea {
  id: mouseArea

  property Item editor
  property int __xWhenPressed
  property int __yWhenPressed
  property bool __movingCursor
  property bool __openingMenu

  function showMenu() {
    if (!__movingCursor) {
      __openingMenu = true
      editMenu.y = editor.cursorRectangle.y - editMenu.height
      editMenu.state = "dropDown"
    }
  }

  acceptedButtons: Qt.LeftButton | Qt.RightButton

  Row {
    id: editMenu
    visible: false
    x: 0
    z: 0

    Button {
      text: qsTr("Select All")
      visible: editor.selectedText === ""
      onClicked: {
        editor.selectAll()
        editMenu.state = ""
      }
    }
    Button {
      text: qsTr("Cut")
      visible: editor.selectedText !== ""
      onClicked: {
        editor.cut()
        editMenu.state = ""
      }
    }
    Button {
      text: qsTr("Copy")
      visible: editor.selectedText !== ""
      onClicked: {
        editor.copy()
        editMenu.state = ""
      }
    }
    Button {
      text: qsTr("Paste")
      visible: editor.canPaste
      onClicked: {
        editor.paste()
        editMenu.state = ""
      }
    }

    states: State {
      name: "dropDown"
      ParentChange {
        target: editMenu
        parent: root
      }
      PropertyChanges {
        target: editMenu
        visible: true
        z: 100
      }
    }
  }

  onEditorChanged: {
    if (editor) {
      editor.visibleChanged.connect(function() {
        if (editMenu && (!mouseArea || !editor || !editor.visible))
          editMenu.state = ""
      })
    }
  }

  preventStealing: true
  onPressed: {
    editor.forceActiveFocus()
    __movingCursor = false
    __openingMenu = false
    if (!cursorHandle.handlePressed(mouse.x, mouse.y) &&
        !selectionHandle.handlePressed(mouse.x, mouse.y)) {
      editMenu.state = ""
      var pos = mapToItem(editor, mouse.x, mouse.y)
      __xWhenPressed = pos.x
      __yWhenPressed = pos.y
    }
  }
  onPositionChanged: {
    if (!cursorHandle.handlePositionChanged(mouse.x, mouse.y) &&
        !selectionHandle.handlePositionChanged(mouse.x, mouse.y)) {
      if (!__movingCursor) {
        __movingCursor = true
        editor.cursorPosition =
            editor.positionAt(__xWhenPressed, __yWhenPressed)
      }
      var pos = mapToItem(editor, mouse.x, mouse.y)
      editor.moveCursorSelection(
            editor.positionAt(pos.x, pos.y))
    }
  }
  onPressAndHold: {
    if (!cursorHandle.handlePressAndHold() &&
        !selectionHandle.handlePressAndHold()) {
      showMenu()
    }
  }
  onReleased: {
    if (!cursorHandle.handleReleased() &&
        !selectionHandle.handleReleased()) {
      if (mouse.button === Qt.RightButton) {
        showMenu()
      } else {
        if (!__movingCursor && !__openingMenu) {
          editor.cursorPosition =
              editor.positionAt(__xWhenPressed, __yWhenPressed)
        }
      }
    }
  }
  onDoubleClicked: {
    __movingCursor = true
    editor.selectWord()
  }

  TextHandle {
    id: selectionHandle
    edit: editor
    area: mouseArea
    leftSide: true
    targetPosition: editor.selectionStart
    onPositionChanged: editor.select(position, editor.selectionEnd)
  }

  TextHandle {
    id: cursorHandle
    edit: editor
    area: mouseArea
    leftSide: false
    targetPosition: editor.selectionEnd
    onPositionChanged: editor.select(editor.selectionStart, position)
  }
}

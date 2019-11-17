/**
 * \file MapEditPage.qml
 * Page to edit a string->string map.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Mar 2019
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

StringListEditPage {
  id: page

  readonly property string mapSeparator: String.fromCharCode(0x00a0) +
                       String.fromCharCode(0x2192) + String.fromCharCode(0x00a0)

  editDialog: keyValueEditDialog

  function addElement(element) {
    // Insert in sorted order
    var idx = 0
    while (idx < model.count &&
           model.get(idx).name < element.name) {
      ++idx
    }
    model.insert(idx, element)
    currentIndex = idx
  }

  function setElements(map) {
    model.clear()
    for (var i = 0, len = map.length; i < len - 1; i += 2) {
      model.append({"name": map[i] + mapSeparator + map[i + 1]})
    }
  }

  function getElements() {
    var map = []
    for (var i = 0; i < model.count; i++) {
      var s = model.get(i).name
      var sepPos = s.indexOf(mapSeparator)
      if (sepPos !== -1) {
        map.push(s.substring(0, sepPos),
                 s.substring(sepPos + mapSeparator.length))
      }
    }
    return map
  }

  Dialog {
    id: keyValueEditDialog

    signal completed(bool ok)

    function setElement(element) {
      var text = element.name
      var keyValue = text.split(mapSeparator)
      keyLineEdit.text = keyValue[0]
      valueLineEdit.text = keyValue[1] || ""
    }

    function getElement() {
      return {name: keyLineEdit.text + mapSeparator + valueLineEdit.text}
    }

    modal: true
    width: Math.min(root.width, constants.gu(70))
    x: (root.width - width) / 2
    y: 0
    standardButtons: Dialog.Ok | Dialog.Cancel

    RowLayout {
      width: parent.width
      TextField {
        id: keyLineEdit
        selectByMouse: true
        Layout.fillWidth: true
      }
      Label {
        text: mapSeparator
      }
      TextField {
        id: valueLineEdit
        selectByMouse: true
        Layout.fillWidth: true
      }
    }

    onAccepted: completed(true)
    onRejected: completed(false)
  }
}

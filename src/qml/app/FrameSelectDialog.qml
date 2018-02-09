/**
 * \file FileSelectDialog.qml
 * Dialog to select a frame.
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

import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

Dialog {
  id: page

  signal frameSelected(string name);

  modal: true
  x: (root.width - width) / 2
  y: root.height / 6
  standardButtons: Dialog.Ok | Dialog.Cancel

  title: qsTr("Add Frame")

  function openFrameNames(frameNames) {
    frameSelectList.model = frameNames
    page.open()
  }

  ColumnLayout {
    Label {
      text: qsTr("Select the frame ID")
    }

    ListView {
      id: frameSelectList
      width: constants.gu(30)
      height: Math.min(constants.gu(35),
                    root.height - 3 * constants.rowHeight - 4 * constants.margins)

      clip: true
      delegate: Standard {
        text: modelData
        highlighted: ListView.view.currentIndex === index
        onClicked: ListView.view.currentIndex = index
        background: Rectangle {
          color: highlighted ? constants.palette.highlight : "transparent"
        }
      }
    }
  }

  onRejected: {
    page.close()
    page.frameSelected("")
  }
  onAccepted: {
    page.close()
    page.frameSelected(frameSelectList.currentItem.text)
  }
}

/**
 * \file FileSelectDialog.qml
 * Dialog to select a frame.
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
import "ComponentsQtQuick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu

Dialog {
  id: page

  signal frameSelected(string name);

  title: qsTr("Add Frame")
  text: qsTr("Select the frame ID")

  function open(frameNames) {
    frameSelectList.model = frameNames
    page.show()
  }

  ListView {
    id: frameSelectList
    height: constants.gu(35)

    clip: true
    delegate: Standard {
      text: modelData
      selected: ListView.view.currentIndex === index
      onClicked: ListView.view.currentIndex = index
    }
  }

  Row {
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
        page.frameSelected("")
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("OK")
      onClicked: {
        page.hide()
        page.frameSelected(frameSelectList.currentItem.text)
      }
    }
  }
}

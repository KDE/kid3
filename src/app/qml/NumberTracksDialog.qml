/**
 * \file NumberTracksDialog.qml
 * Number tracks dialog.
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
import Kid3 1.0

Dialog {
  id: page

  title: qsTr("Number Tracks")

  Label {
    text: qsTr("Start number:")
    width: parent.labelWidth
  }
  TextField {
    id: startNumberEdit
    text: "1"
  }
  Label {
    text: qsTr("Destination:")
    width: parent.labelWidth
  }
  ComboBox {
    id: destinationComboBox
    dropDownParent: page
    width: parent.valueWidth
    model: [ qsTr("Tag 1"),
             qsTr("Tag 2"),
             qsTr("Tag 1 and Tag 2") ]
    function getTagVersion() {
      return [ Frame.TagV1, Frame.TagV2, Frame.TagV2V1 ][currentIndex]
    }
  }
  Row {
    id: totalRow
    spacing: constants.spacing
    CheckBox {
      id: totalCheckBox
      checked: configs.tagConfig().enableTotalNumberOfTracks
    }
    Label {
      height: totalRow.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Total number of tracks")
    }
  }
  TextField {
    id: totalEdit
  }

  Row {
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("OK")
      onClicked: {
        var startNr = parseInt(startNumberEdit.text)
        if (!isNaN(startNr)) {
          var total = totalCheckBox.checked ? parseInt(totalEdit.text) : -1
          if (isNaN(total)) {
            total = -1
          }
          app.numberTracks(startNr, total,
                       script.toTagVersion(destinationComboBox.getTagVersion()))
        }
        page.hide()
      }
    }
  }

  onVisibleChanged: if (visible) {
    totalEdit.text = app.getTotalNumberOfTracksInDir()
  }
}

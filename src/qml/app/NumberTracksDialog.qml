/**
 * \file NumberTracksDialog.qml
 * Number tracks dialog.
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

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import Kid3 1.1 as Kid3

Dialog {
  id: page

  title: qsTr("Number Tracks")
  standardButtons: Dialog.Ok | Dialog.Cancel
  modal: true
  x: (parent.width - width) / 2
  y: parent.height / 6

  ColumnLayout {
  Row {
    spacing: constants.spacing
    CheckBox {
      id: numberCheckBox
      checked: true
    }
    Label {
      height: totalRow.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Start number:")
    }
  }
  TextField {
    id: startNumberEdit
    text: "1"
    selectByMouse: true
  }
  Label {
    text: qsTr("Destination:")
    width: parent.labelWidth
  }
  ComboBox {
    id: destinationComboBox
    width: parent.valueWidth
    model: [ qsTr("Tag 1"),
             qsTr("Tag 2"),
             qsTr("Tag 3"),
             qsTr("Tag 1 and Tag 2"),
             qsTr("All Tags") ]
    function getTagVersion() {
      return [ Kid3.Frame.TagV1, Kid3.Frame.TagV2, Kid3.Frame.TagV3,
               Kid3.Frame.TagV2V1, Kid3.Frame.TagVAll ][currentIndex]
    }
  }
  Row {
    spacing: constants.spacing
    CheckBox {
      id: resetCounterCheckBox
      checked: true
    }
    Label {
      height: totalRow.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Reset counter for each folder")
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
    selectByMouse: true
  }
  }

  onAccepted: {
    var startNr = parseInt(startNumberEdit.text)
    if (!isNaN(startNr)) {
      var total = totalCheckBox.checked ? parseInt(totalEdit.text) : -1
      if (isNaN(total)) {
        total = -1
      }
      var options = 0
      if (numberCheckBox.checked)
        options |= Kid3.Kid3Application.NumberTracksEnabled
      if (resetCounterCheckBox.checked)
        options |= Kid3.Kid3Application.NumberTracksResetCounterForEachDirectory
      app.numberTracks(startNr, total,
                   script.toTagVersion(destinationComboBox.getTagVersion()),
                   options)
    }
  }

  onVisibleChanged: if (visible) {
    totalEdit.text = app.getTotalNumberOfTracksInDir()
  }
}

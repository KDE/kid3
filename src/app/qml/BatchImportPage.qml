/**
 * \file BatchImportPage.qml
 * Batch import page.
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
import Kid3App 1.0

Page {
  id: page

  title: qsTr("Automatic Import")

  Connections {
    target: app.batchImporter
    onReportImportEvent: {
      var str
      switch (type) {
      case BatchImporter.ReadingDirectory:
        str = qsTr("Reading Directory")
        break
      case BatchImporter.Started:
        str = qsTr("Started")
        break
      case BatchImporter.SourceSelected:
        str = qsTr("Source")
        break
      case BatchImporter.QueryingAlbumList:
        str = qsTr("Querying")
        break
      case BatchImporter.FetchingTrackList:
      case BatchImporter.FetchingCoverArt:
        str = qsTr("Fetching")
        break
      case BatchImporter.TrackListReceived:
        str = qsTr("Data received")
        break
      case BatchImporter.CoverArtReceived:
        str = qsTr("Cover")
        break
      case BatchImporter.Finished:
        str = qsTr("Finished")
        break
      case BatchImporter.Aborted:
        str = qsTr("Aborted")
        break
      case BatchImporter.Error:
        str = qsTr("Error")
        break
      }
      if (text) {
        str += ": "
        str += text
      }
      str += "\n"
      textArea.text += str
    }
  }

  Grid {
    id: profileRow
    property int labelWidth: constants.gu(10)
    property int valueWidth: width - labelWidth -spacing
    anchors {
      left: parent.left
      right: parent.right
      top: parent.top
      margins: constants.margins
    }
    columns: 2
    spacing: constants.spacing
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

    Label {
      width: parent.labelWidth
      height: profileComboBox.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Profile:")
    }
    ComboBox {
      id: profileComboBox
      width: parent.valueWidth
      dropDownParent: page
      model: configs.batchImportConfig().profileNames
      currentIndex: configs.batchImportConfig().profileIndex
    }
  }

  TextArea {
    id: textArea
    anchors {
      left: parent.left
      right: parent.right
      top: profileRow.bottom
      bottom: buttonRow.top
      margins: constants.margins
    }
    readOnly: true
    selectByMouse: false
  }
  Row {
    id: buttonRow
    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      margins: constants.margins
    }
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Close")
      onClicked: {
        pageStack.pop()
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Start")
      onClicked: {
        textArea.text = ""
        app.batchImport(profileComboBox.currentText,
                        script.toTagVersion(destinationComboBox.getTagVersion()))
      }
    }
  }

  onActiveChanged: {
    if (active) {
      textArea.text = ""
    } else {
      app.batchImporter.abort()
    }
  }
}

/**
 * \file BatchImportPage.qml
 * Batch import page.
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

Page {
  id: page

  title: qsTr("Automatic Import")

  Connections {
    target: app.batchImporter
    onReportImportEvent: {
      var str
      switch (type) {
      case Kid3.BatchImporter.ReadingDirectory:
        str = qsTr("Reading Folder")
        break
      case Kid3.BatchImporter.Started:
        str = qsTr("Started")
        break
      case Kid3.BatchImporter.SourceSelected:
        str = qsTr("Source")
        break
      case Kid3.BatchImporter.QueryingAlbumList:
        str = qsTr("Querying")
        break
      case Kid3.BatchImporter.FetchingTrackList:
      case Kid3.BatchImporter.FetchingCoverArt:
        str = qsTr("Fetching")
        break
      case Kid3.BatchImporter.TrackListReceived:
        str = qsTr("Data received")
        break
      case Kid3.BatchImporter.CoverArtReceived:
        str = qsTr("Cover")
        break
      case Kid3.BatchImporter.Finished:
        str = qsTr("Finished")
        break
      case Kid3.BatchImporter.Aborted:
        str = qsTr("Aborted")
        break
      case Kid3.BatchImporter.Error:
        str = qsTr("Error")
        break
      }
      if (text) {
        str += ": "
        str += text
      }
      str += "\n"
      textArea.text += str
      textArea.cursorPosition = textArea.text.length
    }
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
      anchors.right: startButton.left
      anchors.verticalCenter: parent.verticalCenter
      clip: true
      text: page.title
    }
    ToolButton {
      id: startButton
      anchors.right: parent.right
      anchors.margins: constants.margins
      text: qsTr("Start")
      onClicked: {
        textArea.text = ""
        app.batchImport(profileComboBox.currentText,
                        script.toTagVersion(destinationComboBox.getTagVersion()))
      }
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
      readonly property var tagVersions: [
        Kid3.Frame.TagV1, Kid3.Frame.TagV2, Kid3.Frame.TagV3,
        Kid3.Frame.TagV2V1, Kid3.Frame.TagVAll
      ]
      width: parent.valueWidth
      model: [ qsTr("Tag 1"),
               qsTr("Tag 2"),
               qsTr("Tag 3"),
               qsTr("Tag 1 and Tag 2"),
               qsTr("All Tags") ]
      currentIndex: tagVersions.indexOf(configs.batchImportConfig().importDest)
      function getTagVersion() {
        return tagVersions[currentIndex]
      }
    }

    Label {
      id: profileLabel
      width: parent.labelWidth
      height: profileComboBox.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Profile:")
    }
    RowLayout {
      width: parent.valueWidth
      IconButton {
        iconName: "edit"
        color: profileLabel.color
        onClicked: {
          editProfilesPage.currentIndex = profileComboBox.currentIndex
          page.StackView.view.push(editProfilesPage)
        }

        ImportProfilesEditPage {
          id: editProfilesPage
          visible: false
          onFinished: {
            profileComboBox.currentIndex = currentIndex
            profileComboBox.currentIndexChanged()
          }
        }
      }
      ComboBox {
        id: profileComboBox
        Layout.fillWidth: true
        model: configs.batchImportConfig().profileNames
        currentIndex: configs.batchImportConfig().profileIndex
      }
    }
  }

  ScrollView {
    id: flick
    anchors {
      left: parent.left
      right: parent.right
      top: profileRow.bottom
      bottom: parent.bottom
      margins: constants.margins
    }

    TextArea {
      id: textArea
      readOnly: true
      selectByMouse: false
    }
  }

  StackView.onActivated: textArea.text = ""
  StackView.onDeactivated: {
    app.batchImporter.abort()
    configs.batchImportConfig().importDest = destinationComboBox.getTagVersion()
    configs.batchImportConfig().profileIndex = profileComboBox.currentIndex
  }
}

/**
 * \file FileList.qml
 * List of files in current directory.
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
import "../componentsqtquick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3 1.0

RaisableRectangle {
  function currentFilePath() {
    return fileModel.getDataValue(fileModel.currentRow,
                                  "filePath")
  }

  Row {
    id: fileButtonRow
    anchors.left: parent.left
    anchors.top: parent.top
    spacing: constants.spacing
    Button {
      id: parentDirButton
      iconName: "go-up"
      width: height
      onClicked: confirmedOpenDirectory(
                   script.getIndexRoleData(fileModel.parentModelIndex(),
                                           "filePath"))
    }
    Button {
      property bool selectAll: true
      iconName: "select"
      width: height
      onClicked: {
        if (selectAll) {
          app.selectAllFiles()
        } else {
          app.deselectAllFiles()
        }
        selectAll = !selectAll
      }
    }
    Button {
      iconName: "go-previous"
      width: height
      onClicked: app.previousFile()
    }
    Button {
      iconName: "go-next"
      width: height
      onClicked: app.nextFile()
    }
  }

  ListView {
    id: fileList

    anchors.left: parent.left
    anchors.top: fileButtonRow.bottom
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    anchors.margins: constants.margins
    clip: true

    model: CheckableListModel {
      id: fileModel
      sourceModel: app.fileProxyModel
      selectionModel: app.fileSelectionModel
      rootIndex: app.fileRootIndex
      onCurrentRowChanged: {
        fileList.currentIndex = row
      }
    }

    delegate: Standard {
      id: fileDelegate
      progression: isDir
      onClicked: {
        if (!isDir) {
          ListView.view.currentIndex = index
          fileModel.currentRow = index
        } else {
          confirmedOpenDirectory(filePath)
        }
      }
      selected: ListView.isCurrentItem
      Row {
        anchors.fill: parent

        CheckBox {
          id: checkField
          anchors.verticalCenter: parent.verticalCenter
          onClicked: {
            // QTBUG-7932, assigning is not possible
            fileModel.setDataValue(index, "checkState",
                                   checked ? Qt.Checked : Qt.Unchecked)
          }
        }
        Binding {
          // workaround for QTBUG-31627
          // should work with "checked: checkState === Qt.Checked"
          target: checkField
          property: "checked"
          value: checkState === Qt.Checked
        }
        Rectangle {
          id: fileImage
          anchors.verticalCenter: parent.verticalCenter
          color: truncated ? constants.errorColor : "transparent"
          width: 16
          height: 16
          Image {
            anchors.fill: parent
            source: "image://kid3/fileicon/" + iconId
          }
        }
        Label {
          id: fileText
          anchors.verticalCenter: parent.verticalCenter
          text: fileName
          color: selected
            ? constants.selectedTextColor : constants.backgroundTextColor
        }
      }
    }
  }
}

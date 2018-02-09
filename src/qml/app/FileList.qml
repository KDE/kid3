/**
 * \file FileList.qml
 * List of files in current directory.
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
import QtQuick.Controls 2.2
import Kid3 1.1 as Kid3

Rectangle {
  id: fileList
  property alias actionButtons: fileButtonRow.control

  signal fileActivated

  function currentFilePath() {
    return fileModel.getDataValue(fileModel.currentRow,
                                  "filePath")
  }

  function parentFilePath() {
    return script.getIndexRoleData(fileModel.parentModelIndex(),
                                   "filePath")
  }

  Item {
    id: fileButtonRow
    property Item control
    width: control ? control.width : undefined
    height: control ? control.height : undefined
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.topMargin: constants.margins
    anchors.leftMargin: constants.margins
    onControlChanged: {
      if (control) control.parent = fileButtonRow
    }
  }

  ListView {
    id: fileListView

    anchors.left: parent.left
    anchors.top: fileButtonRow.control ? fileButtonRow.bottom : parent.top
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    anchors.margins: constants.margins
    clip: true

    model: Kid3.CheckableListModel {
      id: fileModel
      sourceModel: app.fileProxyModel
      selectionModel: app.fileSelectionModel
      rootIndex: app.fileRootIndex
      onCurrentRowChanged: {
        fileListView.currentIndex = row
      }
    }

    delegate: Standard {
      id: fileDelegate
      progression: isDir
      onClicked: {
        if (!isDir) {
          ListView.view.currentIndex = index
          fileModel.currentRow = index
          fileList.fileActivated()
        } else {
          confirmedOpenDirectory(filePath)
        }
      }
      highlighted: ListView.isCurrentItem
      background: Rectangle {
        color: highlighted ? constants.palette.highlight : "transparent"
      }

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
          width: constants.gu(2)
          height: constants.gu(2)
          Image {
            source: iconId == "modified"
                    ? "../icons/modified.svg"
                    : "image://kid3/fileicon/" + (iconId || "null")
            sourceSize.width: parent.width
            sourceSize.height: parent.height
          }
        }
        Label {
          id: fileText
          anchors.verticalCenter: parent.verticalCenter
          text: fileName
          color: fileDelegate.highlighted
            ? constants.palette.highlightedText : constants.palette.text
        }
      }
    }
  }
}

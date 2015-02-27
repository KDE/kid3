/**
 * \file FilterPage.qml
 * Filter page.
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

Page {
  id: page

  title: qsTr("Filter")

  Connections {
    target: app
    onFileFiltered: {
      var str
      switch (type) {
      case FileFilter.Started:
        str = qsTr("Started")
        break
      case FileFilter.Directory:
        str = " "
        str += fileName
        break
      case FileFilter.ParseError:
        str = "parse error"
        break
      case FileFilter.FilePassed:
        str = "+ "
        str += fileName
        break
      case FileFilter.FileFilteredOut:
        str = "- "
        str += fileName
        break
      case FileFilter.Finished:
        str = qsTr("Finished")
        break
      case FileFilter.Aborted:
        str = qsTr("Aborted")
        break
      }
      str += "\n"
      textArea.text += str
    }
  }

  Grid {
    id: filterGrid
    property int labelWidth: Math.max(filterLabel.implicitWidth,
                                      expressionLabel.implicitWidth)
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
      id: filterLabel
      width: parent.labelWidth
      height: filterComboBox.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Filter:")
    }
    ComboBox {
      id: filterComboBox
      width: parent.valueWidth
      dropDownParent: page
      model: configs.filterConfig().filterNames
      currentIndex: configs.filterConfig().filterIndex
    }
    Label {
      id: expressionLabel
      width: parent.labelWidth
      height: expressionEdit.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Expression:")
    }
    TextField {
      id: expressionEdit
      width: parent.valueWidth
      text: configs.filterConfig().filterExpressions[filterComboBox.currentIndex]
    }
  }

  TextArea {
    id: textArea
    anchors {
      left: parent.left
      right: parent.right
      top: filterGrid.bottom
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
      text: qsTr("Apply")
      onClicked: {
        textArea.text = ""
        app.applyFilter(expressionEdit.text)
      }
    }
  }

  onActiveChanged: {
    if (active) {
      textArea.text = ""
    } else {
      app.abortFilter()
    }
  }
}

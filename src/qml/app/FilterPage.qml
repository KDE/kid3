/**
 * \file FilterPage.qml
 * Filter page.
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

Page {
  id: page

  title: qsTr("Filter")

  Connections {
    target: app
    onFileFiltered: {
      var str
      switch (type) {
      case Kid3.FileFilter.Started:
        str = qsTr("Started")
        break
      case Kid3.FileFilter.Directory:
        str = " "
        str += fileName
        break
      case Kid3.FileFilter.ParseError:
        str = "parse error"
        break
      case Kid3.FileFilter.FilePassed:
        str = "+ "
        str += fileName
        break
      case Kid3.FileFilter.FileFilteredOut:
        str = "- "
        str += fileName
        break
      case Kid3.FileFilter.Finished:
        str = qsTr("Finished")
        break
      case Kid3.FileFilter.Aborted:
        str = qsTr("Aborted")
        break
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
        app.applyFilter(expressionEdit.text)
      }
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
      selectByMouse: true
    }
  }

  ScrollView {
    id: flick
    anchors {
      left: parent.left
      right: parent.right
      top: filterGrid.bottom
      bottom: parent.bottom
      margins: constants.margins
    }

    TextArea {
      id: textArea
      readOnly: true
      selectByMouse: false
    }
  }

  StackView.onActivated: {
    textArea.text = ""
  }
  StackView.onDeactivated: {
    app.abortFilter()
  }
}

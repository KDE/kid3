/**
 * \file RenameDirectoryPage.qml
 * Rename directory page.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015-2019  Urs Fleisch
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
import Kid3 1.1 as Kid3

Page {
  id: page

  property int tagMask: Kid3.Frame.TagV2V1
  property string format
  property bool create: false
  property variant formats

  title: qsTr("Rename Folder")

  Connections {
    target: app.dirRenamer
    onActionScheduled: {
      var str = ""
      for (var i = 0; i < actionStrs.length; ++i) {
        if (i > 0)
          str += "  "
        str += actionStrs[i]
        str += "\n"
      }
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
        var errorMsg = app.performRenameActions()
        if (errorMsg) {
          textArea.text += qsTr("Error") + ": " + errorMsg
          textArea.cursorPosition = textArea.text.length
        } else {
          page.refreshPreview()
        }
      }
    }
  }

  Grid {
    id: optionsGrid
    property int labelWidth: Math.max(actionLabel.implicitWidth,
                                      sourceLabel.implicitWidth,
                                      formatLabel.implicitWidth)
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
      id: actionLabel
      width: parent.labelWidth
      height: actionComboBox.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Action:")
    }
    ComboBox {
      id: actionComboBox
      width: parent.valueWidth
      model: [ qsTr("Rename Folder"), qsTr("Create Folder") ]
      onCurrentIndexChanged: {
        page.create = currentIndex === 1
        page.refreshPreview()
      }
    }
    Label {
      id: sourceLabel
      width: parent.labelWidth
      height: sourceComboBox.height
      text: qsTr("Source:")
    }
    ComboBox {
      id: sourceComboBox
      width: parent.valueWidth
      model: [ qsTr("From Tag 2 and Tag 1"),
               qsTr("From Tag 1"),
               qsTr("From Tag 2") ]
      onCurrentIndexChanged: {
        page.tagMask = [ Kid3.Frame.TagV2V1, Kid3.Frame.TagV1, Kid3.Frame.TagV2 ][currentIndex] || 3
        page.refreshPreview()
      }
    }
    Label {
      id: formatLabel
      width: parent.labelWidth
      height: formatComboBox.height
      text: qsTr("Format:")
    }
    RowLayout {
      width: parent.valueWidth
      IconButton {
        iconName: "edit"
        color: formatLabel.color
        onClicked: page.StackView.view.push(editDirFormatsPage)

        StringListEditPage {
          id: editDirFormatsPage
          title: qsTr("Folder Name from Tag")
          visible: false
          StackView.onActivated: {
            setElements(page.formats)
            currentIndex = formatComboBox.currentIndex
          }
          StackView.onDeactivated: {
            page.formats = getElements()
            formatComboBox.currentIndex = currentIndex
            formatComboBox.currentIndexChanged()
          }
        }
      }
      ComboBox {
        id: formatComboBox
        Layout.fillWidth: true
        model: page.formats
        onCurrentIndexChanged: {
          page.format = model[currentIndex]
          page.refreshPreview()
        }
      }
    }
  }

  Label {
    id: previewLabel
    anchors {
      left: parent.left
      right: parent.right
      top: optionsGrid.bottom
      margins: constants.margins
    }
    text: qsTr("Preview")
  }
  ScrollView {
    id: flick
    anchors {
      left: parent.left
      right: parent.right
      top: previewLabel.bottom
      bottom: parent.bottom
      margins: constants.margins
    }

    TextArea {
      id: textArea
      readOnly: true
      selectByMouse: false
    }
  }

  function refreshPreview() {
    textArea.text = ""
    app.renameDirectory(script.toTagVersion(page.tagMask),
                        page.format, page.create)
  }

  StackView.onActivated: {
    refreshPreview()
  }
  StackView.onDeactivated: {
    app.dirRenamer.abort()
    configs.renDirConfig().dirFormats = formats
    configs.renDirConfig().dirFormat = format
  }

  Component.onCompleted: {
    var defaultFormats = configs.renDirConfig().dirFormats
    format = configs.renDirConfig().dirFormat
    var idx = defaultFormats.indexOf(format)
    if (idx === -1) {
      idx = defaultFormats.length
      defaultFormats.push(format)
    }
    formats = defaultFormats
    formatComboBox.currentIndex = idx
  }
}

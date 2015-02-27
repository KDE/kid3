/**
 * \file RenameDirectoryPage.qml
 * Rename directory page.
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
import Kid3 1.0

Page {
  id: page

  property int tagMask: Frame.TagV2V1
  property string format
  property bool create: false
  property variant formats

  title: qsTr("Rename Directory")

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
      dropDownParent: page
      width: parent.valueWidth
      model: [ qsTr("Rename Directory"), qsTr("Create Directory") ]
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
      dropDownParent: page
      width: parent.valueWidth
      model: [ qsTr("From Tag 2 and Tag 1"),
               qsTr("From Tag 1"),
               qsTr("From Tag 2") ]
      onCurrentIndexChanged: {
        page.tagMask = [ Frame.TagV2V1, Frame.TagV1, Frame.TagV2 ][currentIndex]
        page.refreshPreview()
      }
    }
    Label {
      id: formatLabel
      width: parent.labelWidth
      height: formatComboBox.height
      text: qsTr("Format:")
    }
    ComboBox {
      id: formatComboBox
      dropDownParent: page
      width: parent.valueWidth
      model: page.formats
      onCurrentIndexChanged: {
        page.format = model[currentIndex]
        page.refreshPreview()
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
  TextArea {
    id: textArea
    anchors {
      left: parent.left
      right: parent.right
      top: previewLabel.bottom
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
      text: qsTr("Cancel")
      onClicked: {
        pageStack.pop()
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("OK")
      onClicked: {
        var errorMsg = app.performRenameActions()
        if (errorMsg) {
          console.debug("Rename error: " + errorMsg)
        }
        pageStack.pop()
      }
    }
  }

  function refreshPreview() {
    textArea.text = ""
    app.renameDirectory(script.toTagVersion(page.tagMask),
                        page.format, page.create)
  }

  onActiveChanged: {
    if (active) {
      refreshPreview()
    } else {
      app.dirRenamer.abort()
    }
  }

  Component.onCompleted: {
    var defaultFormats = configs.renDirConfig().getDefaultDirFormatList()
    format = configs.renDirConfig().dirFormat
    if (defaultFormats.indexOf(format) === -1) {
      defaultFormats.push(format)
    }
    formats = defaultFormats
  }
}

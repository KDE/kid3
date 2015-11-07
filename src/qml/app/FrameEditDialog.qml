/**
 * \file FrameEditDialog.qml
 * Dialog to edit frames.
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

Dialog {
  id: page

  property QtObject frameObject
  signal frameEdited(variant frame)

  function open(frame) {
    page.title = frame.internalName
    fieldList.model = frame.fields
    frameObject = frame
    page.show()
    if (frame.type === Frame.FT_Picture) {
      app.setCoverArtImageData(frame.getBinaryData())
    }
  }

  Component {
    id: textLineEdit
    TextField {
      text: _modelData.value
      onAccepted: {
        focus = false
      }
      onActiveFocusChanged: {
        if (!activeFocus) {
          _modelData.value = text
        }
      }
    }
  }

  Component {
    id: textEdit
    TextArea {
      text: _modelData.value
      onActiveFocusChanged: {
        if (!activeFocus) {
          _modelData.value = text
        }
      }
    }
  }

  Component {
    id: comboBoxEdit

    ComboBox {
      dropDownParent: root
      model: if (_modelData.id === Frame.ID_TextEnc)
               script.getTextEncodingNames()
             else if (_modelData.id === Frame.ID_PictureType)
               script.getPictureTypeNames()
             else if (_modelData.id === Frame.ID_TimestampFormat)
               script.getTimestampFormatNames()
             else if (_modelData.id === Frame.ID_ContentType)
               script.getContentTypeNames()
      currentIndex: _modelData.value
      onCurrentIndexChanged: _modelData.value = currentIndex
    }
  }

  Component {
    id: exportFileSelectDialog
    FileSelectDialog {
      property variant field
      parent: root
      title: qsTr("Export")
      onFinished: {
        if (path) {
          script.writeFile(path, field.value)
        }
      }
    }
  }

  Component {
    id: importFileSelectDialog
    FileSelectDialog {
      property variant field
      parent: root
      title: qsTr("Import")
      onFinished: {
        if (path) {
          field.value = script.readFile(path)
        }
      }
    }
  }

  Component {
    id: imageView

    Column {
      width: parent.width
      spacing: constants.spacing

      Button {
        id: importButton
        width: parent.width
        text: qsTr("Import")
        onClicked: {
          constants.openPopup(importFileSelectDialog, importButton,
                        {"filePath": app.dirName + "/" +
                                     configs.fileConfig().defaultCoverFileName,
                          "field": _modelData})
        }
      }

      Button {
        id: exportButton
        width: parent.width
        text: qsTr("Export")

        onClicked: {
          constants.openPopup(exportFileSelectDialog, exportButton,
                        {"filePath": app.dirName + "/" +
                                     configs.fileConfig().defaultCoverFileName,
                          "field": _modelData})
        }
      }

      Item {
        id: imageItem
        width: parent.width
        height: 120
        Image {
          anchors.fill: parent
          fillMode: Image.PreserveAspectFit
          source: app.coverArtImageId
          cache: false
        }
      }
    }
  }

  ListView {
    id: fieldList
    clip: true

    // height: someFunction(contentHeight) will report a binding loop,
    // therefore the height is updated manually.
    function updateHeight() {
      height = Math.min(contentHeight, root.height -
                        //3 * constants.rowHeight - //@Ubuntu
                        2 * constants.rowHeight - //@!Ubuntu
                        3 * constants.margins)
    }
    onVisibleChanged: if (visible) updateHeight()
    onContentHeightChanged: updateHeight()

    delegate: Column {
      width: parent.width
      spacing: constants.spacing
      Label {
        id: nameLabel
        text: modelData.name
      }

      Loader {
        width: parent.width
        property QtObject _modelData: modelData
        sourceComponent:
            if (typeof modelData.value === "number")
              if (modelData.id === Frame.ID_TextEnc ||
                  modelData.id === Frame.ID_PictureType ||
                  modelData.id === Frame.ID_TimestampFormat ||
                  modelData.id === Frame.ID_ContentType)
                comboBoxEdit
              else
                textLineEdit
            else if (typeof modelData.value === "string")
              if (modelData.id === Frame.ID_Text)
                textEdit
              else
                textLineEdit
            else if (typeof modelData.value === "object")
              if (modelData.id === Frame.ID_Data &&
                  modelData.type === Frame.FT_Picture)
                imageView
      }
      ThinDivider {
        visible: index != fieldList.count - 1
      }
    }
  }

  Row {
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
        page.frameEdited(null)
        frameObject = null
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("OK")
      onClicked: {
        fieldList.focus = false // to force editingFinished on delegate
        page.hide()
        page.frameEdited(frameObject)
        frameObject = null
      }
    }
  }
}

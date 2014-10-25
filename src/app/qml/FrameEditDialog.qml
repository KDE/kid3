import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

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
        width: 120
        height: 120
        Image {
          sourceSize.width: 120
          sourceSize.height: 120
          source: app.coverArtImageId
          cache: false
        }
      }
    }
  }

  ListView {
    id: fieldList
    height: constants.gu(30)
    clip: true
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

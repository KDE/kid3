import QtQuick 1.1
import Kid3App 1.0

Dialog {
  property string title
  property QtObject frameObject
  signal frameEdited(variant frame)

  function open(frame) {
    __FrameEditDialog_open(frame)
  }

  function reject() {
    __FrameEditDialog_reject()
  }

  function __FrameEditDialog_open(frame) {
    page.title = frame.internalName
    fieldList.model = frame.fields
    frameObject = frame
    __Dialog_open()
    if (frame.type === Frame.FT_Picture) {
      app.setCoverArtImageData(frame.getBinaryData())
    }
  }

  function __FrameEditDialog_reject() {
    __Dialog_reject()
    page.frameEdited(null)
    frameObject = null
  }

  id: page
  width: 400
  height: 400

  Item {
    anchors.fill: parent

    Text {
      id: titleText
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.margins: constants.margins
      height: constants.rowHeight
      text: page.title
    }

    Component {
      id: textLineEdit
      Rectangle {
        width: parent.width
        height: constants.rowHeight
        color: constants.editColor
        TextInput {
          width: parent.width
          anchors.left: parent.left
          anchors.margins: constants.margins
          anchors.verticalCenter: parent.verticalCenter
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
    }

    Component {
      id: textEdit
      Rectangle {
        width: parent.width
        height: 3 * constants.rowHeight
        color: constants.editColor
        TextEdit {
          anchors.fill: parent
          anchors.margins: constants.margins
          wrapMode: TextEdit.Wrap
          text: _modelData.value
          onActiveFocusChanged: {
            if (!activeFocus) {
              _modelData.value = text
            }
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
      id: imageView
      Item {
        width: parent.width
        height: importButton.height + imageItem.height
        anchors.left: parent.left

        Button {
          id: importButton
          width: parent.width
          height: constants.rowHeight
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.topMargin: constants.margins
          text: qsTr("Import")
        }
        Button {
          id: exportButton
          width: parent.width
          height: constants.rowHeight
          anchors.top: importButton.bottom
          anchors.left: parent.left
          anchors.topMargin: constants.margins
          text: qsTr("Export")
        }

        Item {
          id: imageItem
          anchors.top: exportButton.bottom
          anchors.margins: constants.margins
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
      clip: true
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: titleText.bottom
      anchors.bottom: cancelButton.top
      anchors.margins: constants.margins
      delegate: Item {
        width: parent.width
        height: constants.rowHeight
        Text {
          id: nameLabel
          width: 150
          anchors.left: parent.left
          anchors.verticalCenter: parent.verticalCenter
          text: modelData.name
        }
        Loader {
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
          anchors.left: nameLabel.right
          anchors.right: parent.right
        }
      }
    }

    Button {
      id: cancelButton
      anchors.left: parent.left
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins
      width: (parent.width - 2 * constants.margins - constants.spacing) / 2
      text: qsTr("Cancel")
      onClicked: reject()
    }
    Button {
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins
      width: cancelButton.width
      text: qsTr("OK")
      onClicked: {
        fieldList.focus = false // to force editingFinished on delegate
        close()
        page.frameEdited(frameObject)
        frameObject = null
      }
    }
  }
}

import QtQuick 1.1
import Kid3App 1.0

Rectangle {
  property string title
  property QtObject frameObject
  signal frameEdited(variant frame)

  id: page
  width: 400
  height: 400
  border.width: 1
  border.color: "black"
  visible: false
  z: 0

  Item {
    anchors.fill: parent

    Text {
      id: titleText
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.margins: 6
      text: page.title
    }

    Component {
      id: textLineEdit
      TextInput {
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
      id: comboBoxEdit
      ComboBox {
        dropDownParent: fieldList
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

    ListView {
      id: fieldList
      clip: true
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: titleText.bottom
      anchors.bottom: buttonRow.top
      anchors.margins: 6
      delegate: Item {
        width: parent.width
        height: 30
        Text {
          id: nameLabel
          width: 150
          anchors.left: parent.left
          anchors.verticalCenter: parent.verticalCenter
          text: modelData.name
        }
        Loader {
          property QtObject _modelData: modelData
          sourceComponent: if (modelData.id === Frame.ID_TextEnc ||
                               modelData.id === Frame.ID_PictureType ||
                               modelData.id === Frame.ID_TimestampFormat ||
                               modelData.id === Frame.ID_ContentType)
                comboBoxEdit
              else
                textLineEdit
          anchors.left: nameLabel.right
          anchors.right: parent.right
          anchors.verticalCenter: parent.verticalCenter
        }
      }
    }

    Row {
      id: buttonRow
      spacing: 6
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: 6

      Button {
        text: qsTr("Cancel")
        onClicked: {
          page.visible = false
          page.z = 0
          page.frameEdited(null)
          frameObject = null
        }
      }
      Button {
        text: qsTr("OK")
        onClicked: {
          fieldList.focus = false // to force editingFinished on delegate
          page.visible = false
          page.z = 0
          page.frameEdited(frameObject)
          frameObject = null
        }
      }
    }
  }

  function open(frame) {
    page.title = frame.internalName
    fieldList.model = frame.fields
    frameObject = frame
    visible = true
    page.z = 1
  }
}

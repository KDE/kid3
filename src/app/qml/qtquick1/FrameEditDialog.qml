import QtQuick 1.1

Rectangle {
  property string title
  property QtObject frameObject
  signal frameEdited(variant frame)

  id: page
  width: 400
  height: 200
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

    ListView {
      id: fieldList
      clip: true
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: titleText.bottom
      anchors.bottom: buttonRow.top
      anchors.margins: 6
      delegate: Row {
        Text {
          text: model.modelData.name + ":"
        }
        TextInput {
          text: modelData.value
          onAccepted: {
            model.modelData.value = text
          }
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

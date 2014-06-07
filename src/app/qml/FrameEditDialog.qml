import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1

Window {
  property QtObject frameObject
  signal frameEdited(QtObject frame);

  id: page
  width: 400
  height: 200
  visible: false

  Item {
    anchors.fill: parent

    TableView {
      id: fieldList
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.margins: 6
      TableViewColumn {
        role: "name"
        title: "Name"
      }
      TableViewColumn {
        role: "value"
        title: "Value"
        delegate: TextField {
          text: styleData.value
          onEditingFinished: {
            modelData.value = text
          }
        }
      }
    }

    Row {
      spacing: 6
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: 6

      Button {
        text: qsTr("&Cancel")
        onClicked: {
          page.visible = false
          page.frameEdited(null)
          frameObject = null
        }
      }
      Button {
        text: qsTr("&OK")
        onClicked: {
          fieldList.focus = false // to force editingFinished on delegate
          page.visible = false
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
  }
}

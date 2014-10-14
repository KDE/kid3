import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1

Window {
  signal frameSelected(string name);

  id: page
  width: 300
  height: 100

  visible: false
  title: qsTr("Add Frame")

  Item {
    anchors.fill: parent

    Text {
      id: messageText
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.margins: 6
      text: qsTr("Select the frame ID")
    }
    ComboBox {
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: messageText.bottom
      anchors.margins: 6
      id: frameSelectComboBox
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
          page.frameSelected("")
        }
      }
      Button {
        text: qsTr("&OK")
        onClicked: {
          page.visible = false
          page.frameSelected(frameSelectComboBox.currentText)
        }
      }
    }
  }

  function open(frameNames) {
    frameSelectComboBox.model = frameNames
    page.visible = true
  }
}

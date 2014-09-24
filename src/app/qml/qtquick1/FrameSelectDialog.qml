import QtQuick 1.1

Rectangle {
  signal frameSelected(string name);

  id: page
  width: 300
  height: 300
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
      text: qsTr("Add Frame")
    }
    Text {
      id: messageText
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: titleText.bottom
      anchors.margins: 6
      text: qsTr("Select the frame ID")
    }
    ListView {
      id: frameSelectList
      clip: true
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: messageText.bottom
      anchors.bottom: buttonRow.top
      anchors.margins: 6
      delegate: Text {
        text: modelData
        color: ListView.isCurrentItem ? "red" : "black"
        MouseArea {
          anchors.fill: parent
          onClicked: {
            parent.ListView.view.currentIndex = index
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
          page.frameSelected("")
        }
      }
      Button {
        text: qsTr("OK")
        onClicked: {
          page.visible = false
          page.z = 0
          page.frameSelected(frameSelectList.currentItem.text)
        }
      }
    }
  }

  function open(frameNames) {
    frameSelectList.model = frameNames
    page.visible = true
    page.z = 1
  }
}

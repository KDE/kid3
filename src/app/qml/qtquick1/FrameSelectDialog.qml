import QtQuick 1.1

Rectangle {
  signal frameSelected(string name);

  id: page
  width: 300
  height: 300
  color: constants.palette.window
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
      anchors.margins: constants.margins
      text: qsTr("Add Frame")
    }
    Text {
      id: messageText
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: titleText.bottom
      anchors.margins: constants.margins
      text: qsTr("Select the frame ID")
    }
    ListView {
      id: frameSelectList
      clip: true
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: messageText.bottom
      anchors.bottom: buttonRow.top
      anchors.margins: constants.margins
      delegate: Rectangle {
        id: frameSelectDelegate
        color: ListView.isCurrentItem
               ? constants.palette.highlight : constants.palette.window
        width: parent.width
        height: constants.rowHeight
        Text {
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.verticalCenter: parent.verticalCenter
          anchors.margins: constants.margins
          text: modelData
          color: frameSelectDelegate.ListView.isCurrentItem
                 ? constants.palette.highlightedText :constants.palette.text
          MouseArea {
            anchors.fill: parent
            onClicked: {
              frameSelectDelegate.ListView.view.currentIndex = index
            }
          }
        }
      }
    }
    Row {
      id: buttonRow
      spacing: constants.spacing
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins

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
          page.frameSelected(frameSelectList.currentItem.data[0].text)
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

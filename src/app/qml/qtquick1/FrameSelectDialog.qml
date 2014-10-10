import QtQuick 1.1

Dialog {
  id: page

  signal frameSelected(string name);

  function open(frameNames) {
    __FrameSelectDialog_open(frameNames)
  }

  function reject() {
    __FrameSelectDialog_reject()
  }

  function __FrameSelectDialog_open(frameNames) {
    frameSelectList.model = frameNames
    __Dialog_open()
  }

  function __FrameSelectDialog_reject() {
    __Dialog_reject()
    page.frameSelected("")
  }

  width: 300
  height: 300

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
      anchors.bottom: cancelButton.top
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
        close()
        page.frameSelected(frameSelectList.currentItem.data[0].text)
      }
    }
  }
}

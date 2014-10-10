import QtQuick 1.1

Rectangle {
  id: page

  function open() {
    __Dialog_open()
  }

  function close() {
    __Dialog_close()
  }

  function reject() {
    __Dialog_reject()
  }

  function __Dialog_open() {
    page.visible = true
    page.z = 1
  }

  function __Dialog_close() {
    page.visible = false
    page.z = 0
  }

  function __Dialog_reject() {
    __Dialog_close()
  }

  color: constants.palette.window
  border.width: 1
  border.color: "black"
  visible: false
  z: 0

  // Handle mouse clicks inside the dialog
  MouseArea {
    parent: page
    anchors.fill: parent
  }

  Rectangle {
    parent: root
    anchors.fill: parent
    z: page.z - 1
    color: "#80000000"

    // Handle mouse clicks outside the dialog
    MouseArea {
      anchors.fill: parent
      onClicked: reject()
    }
  }
}

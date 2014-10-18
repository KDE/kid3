import QtQuick 2.2

Rectangle {
  id: page

  function show() {
    page.visible = true
    page.z = 1
  }

  function hide() {
    page.visible = false
    page.z = 0
  }

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
    }
  }
}

import QtQuick 2.2

Rectangle {
  id: page

  function show() {
    page.visible = true
    constants.lastPopupZ += 2
    page.z = constants.lastPopupZ
    backgroundArea.z = constants.lastPopupZ - 1
    backgroundArea.visible = true
  }

  function hide() {
    page.visible = false
    if (page.z === constants.lastPopupZ) {
      constants.lastPopupZ -= 2
    }
    page.z -= 2
    backgroundArea.z = -1
    backgroundArea.visible = false
  }

  visible: false
  z: 0

  // Handle mouse clicks inside the dialog
  MouseArea {
    parent: page
    anchors.fill: parent
  }

  Rectangle {
    id: backgroundArea
    parent: root
    anchors.fill: parent
    z: -1
    visible: false
    color: "#80000000"

    // Handle mouse clicks outside the dialog
    MouseArea {
      anchors.fill: parent
    }
  }
}

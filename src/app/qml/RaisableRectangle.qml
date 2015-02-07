import QtQuick 2.2

Rectangle {
  id: rect

  signal clicked

  property bool raised: true

  z: raised ? 1 : 0

  MouseArea {
    z: parent.raised ? 0 : 1

    anchors.fill: parent
    onClicked: {
      rect.clicked()
    }
  }
}

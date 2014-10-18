import QtQuick 2.2

Rectangle {
  property alias title: titleLabel.text
  default property alias contents: contentsItem.data

  anchors.fill: parent
  Text {
    id: titleLabel
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.margins: constants.margins
    height: constants.rowHeight
  }
  Item {
    id: contentsItem
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: titleLabel.bottom
    anchors.bottom: parent.bottom
    anchors.margins: constants.margins
  }
}

import QtQuick 2.2

Rectangle {
  property alias title: titleLabel.text
  default property alias contents: contentsItem.data
  property bool active: visible

  anchors.fill: parent
  Row {
    id: titleRow
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.margins: constants.margins
    height: constants.rowHeight
    Button {
      border.width: 0
      iconName: "go-previous"
      visible: pageStack.canPop
      onClicked: pageStack.pop()
    }
    Text {
      id: titleLabel
      anchors.verticalCenter: parent.verticalCenter
    }
  }
  Item {
    id: contentsItem
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: titleRow.bottom
    anchors.bottom: parent.bottom
    anchors.margins: constants.margins
  }
}

import QtQuick 2.2

Rectangle {
  id: emptyListItem

  property bool selected: false

  signal clicked()

  width: parent.width
  height: constants.rowHeight
  color: selected
         ? constants.palette.highlight : constants.palette.window

  MouseArea {
    anchors.fill: parent
    onClicked: {
      emptyListItem.clicked()
    }
  }
}

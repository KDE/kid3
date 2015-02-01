import QtQuick 2.2

Rectangle {
  id: emptyListItem

  property bool selected: false
  property bool __acceptEvents: true
  property alias __mouseArea: mouseArea

  signal clicked()

  width: parent ? parent.width : constants.gu(31)
  height: constants.rowHeight
  color: selected
         ? constants.palette.highlight : constants.palette.window

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    onClicked: {
      if (emptyListItem.__acceptEvents) {
        emptyListItem.clicked()
      }
    }
  }
}

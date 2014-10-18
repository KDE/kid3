import QtQuick 2.2

Empty {
  id: emptyListItem

  property alias text: textLabel.text
  property bool progression

  Text {
    id: textLabel
    anchors.left: parent.left
    anchors.right: progressionLabel.left
    anchors.verticalCenter: parent.verticalCenter
    anchors.margins: constants.margins
    color: selected
           ? constants.palette.highlightedText :constants.palette.text
  }
  Text {
    id: progressionLabel
    anchors.right: parent.right
    anchors.verticalCenter: parent.verticalCenter
    anchors.margins: constants.margins
    text : ">"
    visible: progression
  }
}

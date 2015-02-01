import QtQuick 2.2

Empty {
  id: listItem
  property alias text: textLabel.text
  property bool progression
  property alias control: controlContainer.control

  __acceptEvents: false

  Text {
    id: textLabel
    anchors.left: parent.left
    anchors.right: controlContainer.left
    anchors.verticalCenter: parent.verticalCenter
    anchors.margins: constants.margins
    color: selected
           ? constants.palette.highlightedText :constants.palette.text
  }
  Item {
    id: controlContainer
    property Item control
    width: control ? control.width : undefined
    height: control ? control.height : undefined
    anchors.right: progressionLabel.left
    anchors.verticalCenter: parent.verticalCenter
    anchors.margins: constants.margins
    onControlChanged: {
      if (control) control.parent = controlContainer;
    }
    Connections {
      target: listItem.__mouseArea

      onClicked: {
        if (control && listItem.__mouseArea.mouseX < progressionLabel.x) {
          if (control.enabled && control.hasOwnProperty("clicked"))
            control.clicked();
        } else {
          listItem.clicked();
        }
      }
    }
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

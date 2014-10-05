import QtQuick 1.1

Item {
  id: dropDown

  property Item dropDownRoot
  property variant model
  property string currentText
  property alias currentIndex: listView.currentIndex

  signal clicked

  height: 0
  z: 0
  clip: true

  function setVisible(enable) {
    state = enable ? "dropDown" : ""
  }

  function toggleVisible() {
    state = state === "dropDown" ? "": "dropDown"
  }

  function calculateHeight() {
    var parentHeight = dropDownRoot.height
    var listViewHeight = listView.count * constants.rowHeight
    return listViewHeight <= parentHeight
        ? listViewHeight : parentHeight
  }

  function calculateY(dropDownHeight) {
    var yPos = dropDown.mapToItem(dropDownRoot, 0, 0).y
    var yOffset = dropDownRoot.height - yPos - dropDownHeight
    if (yOffset < 0)
      yPos += yOffset
    if (yPos < 0)
      yPos = 0
    return yPos
  }

  ListView {
    id: listView
    anchors.fill: parent
    model: dropDown.model
    Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Beginning)

    delegate: Rectangle {
      width: parent.width
      height: constants.rowHeight
      color: ListView.isCurrentItem
             ? constants.palette.highlight : constants.palette.mid
      Text {
        id: delegateText
        color: parent.ListView.isCurrentItem
               ? constants.palette.highlightedText : constants.palette.text
        text: model.modelData || model.display
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: constants.margins
      }
      MouseArea {
        anchors.fill: parent
        onClicked: {
          dropDown.state = ""
          currentText = delegateText.text
          listView.currentIndex = index;
          dropDown.clicked()
        }
      }
    }
  }

  MouseArea {
    id: rootMouseArea
    parent: dropDownRoot
    anchors.fill: parent
    z: dropDown.z - 1
    onClicked: dropDown.state = ""
  }

  states: State {
    name: "dropDown";
    ParentChange {
      target: dropDown
      parent: dropDown.dropDownRoot
      height: dropDown.calculateHeight()
      y: dropDown.calculateY(height)
    }
    PropertyChanges {
      target: dropDown
      z: 100
    }
  }
}

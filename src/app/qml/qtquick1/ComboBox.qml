import QtQuick 1.1

Rectangle {
  id: comboBox
  property variant model: ["Item 1", "Item 2", "Item 3"]
  // Can be set to make sure that the dropDown is on top
  property variant dropDownParent: comboBox
  property alias currentText: chosenItemText.text
  property alias currentIndex: listView.currentIndex
  width: 100
  height: constants.rowHeight
  smooth: true

  function calculateHeight() {
    var parentHeight = dropDownParent.height
    var listViewHeight = listView.count * constants.rowHeight
    return listViewHeight <= parentHeight
        ? listViewHeight : parentHeight
  }

  function calculateY(dropDownHeight) {
    var yPos = dropDown.mapToItem(dropDownParent, 0, 0).y
    var yOffset = dropDownParent.height - yPos - dropDownHeight
    if (yOffset < 0)
      yPos += yOffset
    if (yPos < 0)
      yPos = 0
    return yPos
  }

  Rectangle {
    id: chosenItem
    radius: 4
    width: parent.width
    height: comboBox.height
    color: constants.palette.mid
    smooth: true
    Text {
      anchors.left: parent.left
      anchors.margins: constants.margins
      anchors.verticalCenter: parent.verticalCenter
      id: chosenItemText
      // Check to avoid error if model is not an array
      text: !!comboBox.model && !!comboBox.model.length
            ? model[listView.currentIndex] : ""
      smooth: true
    }

    MouseArea {
      anchors.fill: parent;
      onClicked: {
        comboBox.state = comboBox.state === "dropDown" ? "": "dropDown"
      }
    }
  }

  Item {
    id: dropDown
    width: comboBox.width
    height: 0
    clip: true
    anchors.top: chosenItem.bottom
    anchors.margins: 2

    ListView {
      id: listView
      anchors.fill: parent
      model: comboBox.model
      Component.onCompleted: positionViewAtIndex(currentIndex, ListView.Beginning)

      delegate: Rectangle {
        width: comboBox.width
        height: comboBox.height
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
            comboBox.state = ""
            chosenItemText.text = delegateText.text
            listView.currentIndex = index;
          }
        }
      }
    }
  }

  states: State {
    name: "dropDown";
    ParentChange {
      target: dropDown
      parent: dropDownParent
      height: comboBox.calculateHeight()
      y: comboBox.calculateY(height)
    }
  }
}

import QtQuick 2.2

Item {
  id: comboBox
  property alias model: dropDown.model
  // Can be set to make sure that the dropDown is on top
  property alias dropDownParent: dropDown.dropDownRoot
  property alias currentText: selectedItemText.text
  property alias currentIndex: dropDown.currentIndex

  height: constants.gu(4)

  Rectangle {
    id: selectedItem
    radius: 4
    width: parent.width
    height: parent.height
    anchors.verticalCenter: parent.verticalCenter
    color: constants.comboBoxColor
    smooth: true
    Text {
      id: selectedItemText
      anchors.left: parent.left
      anchors.margins: constants.margins
      anchors.verticalCenter: parent.verticalCenter
      // Check to avoid error if model is not an array
      text: !!comboBox.model && !!comboBox.model.length
            ? model[comboBox.currentIndex] :
              model.hasOwnProperty("get")
              ? model.get(comboBox.currentIndex).display
              : ""

      smooth: true
    }

    MouseArea {
      anchors.fill: parent;
      onClicked: dropDown.toggleVisible()
    }
  }

  DropDownList {
    id: dropDown
    width: comboBox.width
    anchors.top: selectedItem.bottom
    anchors.margins: 2

    onCurrentTextChanged: selectedItemText.text = currentText
  }
}

import QtQuick 1.1

Item {
  id: checkbox

  property string text
  property bool checked

  signal clicked

  width: checkboxText.width + checkField.width + 12
  height: constants.rowHeight

  Rectangle {
    id: checkField
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
    width: 12
    height: 12
    color: constants.palette.base
    border.width: 1
    border.color: "black"

    Text {
      text: checkbox.checked ? "x" : ""
      anchors.centerIn: parent
    }
  }

  Text {
    id: checkboxText
    anchors.left: checkField.right
    anchors.verticalCenter: parent.verticalCenter
    anchors.margins: constants.margins
    text: checkbox.text
  }

  MouseArea {
    anchors.fill: parent
    onClicked: {
      checkbox.checked = !checkbox.checked
      checkbox.clicked()
    }
  }
}

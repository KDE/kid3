import QtQuick 1.1

Item {
  id: checkbox

  property string text
  property bool checked

  signal clicked

  width: checkboxText.width + checkField.width + 12
  height: checkboxText.height + 6

  Rectangle {
    id: checkField
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
    width: 12
    height: 12
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
    anchors.margins: 6
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

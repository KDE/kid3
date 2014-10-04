import QtQuick 1.1

Rectangle {
  id: checkfield

  property bool checked

  signal clicked

  width: 12
  height: 12
  border.width: 1
  border.color: "black"
  color: constants.palette.base

  Text {
    id: checkboxText
    text: checkfield.checked ? "x" : ""
    anchors.centerIn: parent
  }

  MouseArea {
    anchors.fill: parent
    onClicked: {
      checkfield.checked = !checkfield.checked
      checkfield.clicked()
    }
  }
}

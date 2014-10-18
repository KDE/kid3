import QtQuick 2.2

FocusScope {
  id: textField

  property alias text: textInput.text
  signal accepted()

  height: constants.rowHeight
  Rectangle {
    anchors.fill: parent
    color: constants.editColor
    TextInput {
      id: textInput
      anchors {
        left: parent.left
        right: parent.right
        verticalCenter: parent.verticalCenter
        leftMargin: constants.margins
      }
      onAccepted: textField.accepted()
    }
  }
}

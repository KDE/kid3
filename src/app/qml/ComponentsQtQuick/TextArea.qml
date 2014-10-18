import QtQuick 2.2

FocusScope {
  property alias text: textInput.text

  height: 3 * constants.rowHeight
  Rectangle {
    anchors.fill: parent
    color: constants.editColor
    TextEdit {
      id: textInput
      anchors.fill: parent
      anchors.leftMargin: constants.margins
    }
  }
}

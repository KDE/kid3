import QtQuick 2.2

FocusScope {
  property alias text: textInput.text
  property alias readOnly: textInput.readOnly
  property alias selectByMouse: textInput.selectByMouse

  height: 3 * constants.rowHeight
  Rectangle {
    anchors.fill: parent
    color: constants.editColor

    Flickable {
      id: flick

      anchors.fill: parent
      anchors.leftMargin: constants.margins
      contentWidth: textInput.paintedWidth
      contentHeight: textInput.paintedHeight
      flickableDirection: Flickable.VerticalFlick
      clip: true

      function ensureVisible(r) {
        if (contentX >= r.x)
          contentX = r.x;
        else if (contentX+width <= r.x + r.width)
          contentX = r.x + r.width - width;
        if (contentY >= r.y)
          contentY = r.y;
        else if (contentY + height <= r.y + r.height)
          contentY = r.y + r.height - height;
      }

      TextEdit {
        id: textInput
        width: flick.width
        height: flick.height
        focus: true
        wrapMode: TextEdit.Wrap
        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
      }
    }
  }
}

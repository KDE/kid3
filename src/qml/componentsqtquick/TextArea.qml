/**
 * \file TextArea.qml
 * Multiple line text input area.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2

FocusScope {
  property alias text: textInput.text
  property alias readOnly: textInput.readOnly
  property alias selectByMouse: textInput.selectByMouse

  height: 3 * constants.controlHeight
  Rectangle {
    id: rect
    anchors.fill: parent
    color: constants.editColor

    Flickable {
      id: flick

      anchors.fill: parent
      anchors.leftMargin: constants.margins
      // The content width is kept smaller than the textInput width in order
      // to have the right margin not covered by the TextSelectionHandler
      // so that the text can be flicked at the right margin. It must,
      // however, be large enough to grab the selection handles.
      contentWidth: Math.min(textInput.paintedWidth + constants.gu(5),
                             textInput.width)
      contentHeight: Math.max(textInput.paintedHeight + constants.gu(2),
                              textInput.height)
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
        textMargin: constants.spacing
        focus: true
        wrapMode: TextEdit.Wrap
        inputMethodHints: Qt.ImhNoPredictiveText
        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
      }
      TextSelectionHandler {
        editor: textInput
        anchors.fill: parent
      }
    }
  }
}

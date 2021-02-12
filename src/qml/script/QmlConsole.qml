/**
 * \file QmlConsole.qml
 * Simple console to play with Kid3's QML API.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 Mar 2015
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
import QtQml 2.2
import Kid3 1.0

Rectangle {
  id: root

  /**
   * tagv1, tagv2, tagv2v1 can be used for the tag version with both
   * QtQuick 1 (Qt 4) and QtQuick 2 (Qt 5).
   * QtQuick 1 needs the enum types from C++, so a helper function is used.
   */
  //property variant tagv1: script.toTagVersion(Frame.TagV1) //@QtQuick1
  //property variant tagv2: script.toTagVersion(Frame.TagV2) //@QtQuick1
  //property variant tagv2v1: script.toTagVersion(Frame.TagV2V1) //@QtQuick1
  readonly property int tagv1: Frame.TagV1 //@QtQuick2
  readonly property int tagv2: Frame.TagV2 //@QtQuick2
  readonly property int tagv2v1: Frame.TagV2V1 //@QtQuick2

  // An empty context object for interactive variable storage.
  property var ctx: Object.create(null) //@QtQuick2

  property SystemPalette palette: SystemPalette {}
  property string help:
      ".quit    - quit console\n" +
      ".help    - show help\n" +
      ".clear   - clear output\n" +
      "dir(obj) - dump script properties of object\n" +
      "script.properties(obj) - dump Qt properties of object\n"

  width: 400
  height: 300
  color: palette.window

  ScriptUtils {
    id: script
  }

  ConfigObjects {
    id: configs
  }

  Rectangle {
    anchors {
      top: parent.top
      bottom: inputRect.top
      left: parent.left
      right: parent.right
      margins: 8
    }
    border.width: 1
    color: palette.base

    Flickable {
      id: flick
      anchors.fill: parent
      anchors.margins: 8
      contentWidth: output.paintedWidth
      contentHeight: output.paintedHeight
      flickableDirection: Flickable.VerticalFlick
      clip: true

      function ensureVisible(r) {
        if (contentX >= r.x)
          contentX = r.x;
        else if (contentX + width <= r.x + r.width)
          contentX = r.x + r.width - width;
        if (contentY >= r.y)
          contentY = r.y;
        else if (contentY + height <= r.y + r.height)
          contentY = r.y + r.height - height;
      }

      TextEdit {
        id: output
        color: palette.text
        width: flick.width
        height: flick.height
        wrapMode: TextEdit.Wrap
        textFormat: TextEdit.PlainText
        readOnly: true
        selectByMouse: true
        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
        text: help
      }
    }
  }

  Rectangle {
    id: inputRect
    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      margins: 8
    }
    height: input.implicitHeight + 2 * input.anchors.margins
    border.width: 1
    color: palette.base

    TextInput {
      id: input
      color: palette.text

      property variant history: []
      property int historyIndex: 0

      /**
       * Get string with JavaScript properties of object.
       */
      function dir(obj) {
        var lines = []
        for (var p in obj) {
          lines.push(p + " = " +
                     (typeof obj[p] !== "function" ? obj[p] : "function"))
        }
        // Sort and remove duplicates.
        lines.sort()
        lines = lines.filter(function(val, idx, arr) {
          return idx === 0 || val !== arr[idx - 1];
        })
        return lines.join("\n")
      }

      anchors.fill: parent
      anchors.margins: 8
      focus: true
      selectByMouse: true
      clip: true
      onAccepted: {
        if (!text)
          return
        if (text === ".quit") {
          Qt.quit()
        } else if (text === ".clear") {
          output.text = ""
          history = []
        } else if (text === ".help") {
          output.text += help
        } else if (text === "dir") {
          output.text +=
              "Try dir(obj), with obj e.g. app, script, configs\n"
        } else {
          var result
          try {
            result = eval(text)
          } catch (ex) {
            result = ex.message
          }
          if (typeof result === "undefined") {
            result = "undefined"
          } else if (result === null) {
            result = "null"
          }
          output.text += "> " + text + "\n" + result.toString() + "\n"
        }
        output.cursorPosition = output.text.length
        var hist = history
        hist.push(text)
        history = hist
        text = ""
      }
      Keys.onPressed: {
        var histLen
        if (event.key === Qt.Key_Up) {
          histLen = history.length
          if (historyIndex < histLen) {
            text = history[histLen - 1 - historyIndex]
            ++historyIndex
          } else {
            text = ""
          }
        } else if (event.key === Qt.Key_Down) {
          histLen = history.length
          if (historyIndex > 0 && historyIndex <= histLen) {
            --historyIndex
            text = history[histLen - 1 - historyIndex]
          } else {
            text = ""
          }
        } else {
          historyIndex = 0
        }
      }
    }
  }
  Component.onCompleted: {
    if (typeof args === "undefined") {
      // Started as a QML script outside of Kid3.
      app.selectedFilesUpdated.connect(app.tagsToFrameModels)
      app.selectedFilesChanged.connect(app.tagsToFrameModels)
      app.readConfig()
    }
  }
}

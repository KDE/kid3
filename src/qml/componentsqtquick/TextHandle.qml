/**
 * \file TextHandle.qml
 * Handle to change selection with touch screen.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Nov 2015
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

ScaledImage {
  source: "../icons/selection_handle.svg"
  property Item edit
  property Item area
  property int targetPosition
  property int position
  property bool leftSide: false
  property bool __moving: false
  //property variant __offset //@QtQuick1
  property point __offset //@QtQuick2
  // Mention visible to force re-evaluation if selection changes
  //property variant __selectionRectangle: //@QtQuick1
  property rect __selectionRectangle: //@QtQuick2
      edit.positionToRectangle(visible ? targetPosition : 0)
  //property variant __mappedPos: //@QtQuick1
  property var __mappedPos: //@QtQuick2
      edit.mapToItem(area, __selectionRectangle.x, __selectionRectangle.y)

  function handlePressed(xPos, yPos) {
    if (visible && xPos >= x - (leftSide ? constants.gu(3) : 0) &&
        xPos <= x + (leftSide ? width : constants.gu(5)) &&
        yPos >= y && yPos <= y + constants.gu(5)) {
      __moving = true
      position = targetPosition
      var positionRect = edit.positionToRectangle(targetPosition)
      var center = area.mapFromItem(edit,
          positionRect.x + (positionRect.width / 2),
          positionRect.y + (positionRect.height / 2))
      __offset = Qt.point(xPos - center.x, yPos - center.y)
      return true
    } else {
      __moving = false
      return false
    }
  }

  function handlePositionChanged(xPos, yPos) {
    if (__moving) {
      var pt = area.mapToItem(edit, xPos - __offset.x, yPos - __offset.y)
      var pos = edit.positionAt(pt.x, pt.y)
      pos = Math.max(pos, 0)
      //pos = Math.min(pos, edit.text.length) //@QtQuick1
      pos = Math.min(pos, edit.length) //@QtQuick2
      position = pos
      return true
    } else {
      return false
    }
  }

  function handlePressAndHold() {
    return __moving
  }

  function handleReleased() {
    if (__moving) {
      __moving = false
      return true
    } else {
      return false
    }
  }

  visible: edit.selectionStart !== edit.selectionEnd
  x: __mappedPos.x - width / 2
  y: __mappedPos.y + __selectionRectangle.height
}

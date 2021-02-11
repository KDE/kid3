/**
 * \file UiConstants.qml
 * Constants for UI metrics.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015-2018  Urs Fleisch
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

import QtQuick 2.11

QtObject {
  function gu(n) {
    return n * gridUnit
  }

  function openPopup(component, parent, params) {
    var popover
    if (params !== undefined) {
      popover = component.createObject(parent, params)
    } else {
      popover = component.createObject(parent)
    }
    popover.open()
    return popover
  }

  property int gridUnit: 8
  property int titlePixelSize: 14
  property real imageScaleFactor: 1.0
  property int margins: gu(1)
  property int spacing: gu(1)
  property color errorColor: "red"
  property int rowHeight: gu(6)
  property int controlHeight: gu(5)
  property SystemPalette palette: SystemPalette {}
  property color highlightColor: palette.highlight
  property color highlightedTextColor: palette.highlightedText
  property color textColor: palette.text
  property color baseColor: palette.base
}

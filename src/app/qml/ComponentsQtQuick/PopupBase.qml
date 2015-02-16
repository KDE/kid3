/**
 * \file PopupBase.qml
 * Base component for popups.
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

Rectangle {
  id: page

  function show() {
    page.visible = true
    constants.lastPopupZ += 2
    page.z = constants.lastPopupZ
    backgroundArea.z = constants.lastPopupZ - 1
    backgroundArea.visible = true
  }

  function hide() {
    page.visible = false
    if (page.z === constants.lastPopupZ) {
      constants.lastPopupZ -= 2
    }
    page.z -= 2
    backgroundArea.z = -1
    backgroundArea.visible = false
  }

  visible: false
  z: 0

  // Handle mouse clicks inside the dialog
  MouseArea {
    parent: page
    anchors.fill: parent
  }

  Rectangle {
    id: backgroundArea
    parent: root
    anchors.fill: parent
    z: -1
    visible: false
    color: "#80000000"

    // Handle mouse clicks outside the dialog
    MouseArea {
      anchors.fill: parent
    }
  }
}

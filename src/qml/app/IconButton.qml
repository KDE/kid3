/**
 * \file IconButton.qml
 * Tool button with an icon.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Feb 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

import QtQuick 2.9
import QtQuick.Controls 2.2

ToolButton {
  property string iconName
  property alias color: text.color

  Text {
    id: text
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    font.family: materialFont.name
    font.pixelSize: 24
    text: if (iconName) {
              "go-up": "^",
              "select": "S",
              "clear": "X",
              "go-previous": "<",
              "go-next": ">",
              "drawer": "=",
              "navigation-menu": ":",
              "edit": "/",
              "add": "+",
              "remove": "-"
            }[iconName]
  }
}

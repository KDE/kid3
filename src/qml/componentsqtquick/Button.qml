/**
 * \file Button.qml
 * Button.
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

Rectangle  {
  id: container

  property string text
  property string iconName

  signal clicked

  width: buttonLabel.width + 20
  height: constants.controlHeight
  border  { width: 1; color: Qt.darker(constants.palette.button) }
  smooth: true
  radius: 4

  // color the button with a gradient
  gradient: Gradient  {
    GradientStop  {
      position: 0.0
      color:  {
        if (mouseArea.pressed)
          return constants.palette.dark
        else
          return constants.palette.light
      }
    }
    GradientStop  { position: 1.0; color: constants.palette.button }
  }

  MouseArea  {
    id: mouseArea
    anchors.fill: parent
    onClicked: container.clicked();
  }

  Text  {
    id: buttonLabel
    anchors.centerIn: container
    color: constants.palette.buttonText
    text: if (container.text)
            container.text
          else if (iconName === "go-up")
            "^"
          else if (iconName === "select")
            "*"
          else if (iconName === "clear")
            "x"
          else if (iconName === "go-previous")
            "<"
          else if (iconName === "go-next")
            ">"
          else if (iconName === "navigation-menu")
            "="
          else if (iconName === "edit")
            "/"
          else if (iconName === "add")
            "+"
          else if (iconName === "remove")
            "-"
          else
            ""
  }
}

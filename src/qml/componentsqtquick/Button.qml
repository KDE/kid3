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
  property bool transparent: false

  signal clicked

  width: buttonContent.width + 20
  height: constants.controlHeight
  border  {
    width: transparent ? 0 : 1
    color: mouseArea.pressed ? constants.selectedBorderColor
                             : constants.borderColor
  }
  smooth: true
  radius: 2
  color: mouseArea.pressed ? constants.selectedButtonColor
                           : transparent ? "transparent" : constants.buttonColor

  Rectangle {
    visible: mouseArea.pressed
    anchors.fill: parent
    gradient: Gradient {
      GradientStop { position: 0.0; color: "#26000000" }
      GradientStop { position: 0.1; color: "transparent" }
    }
  }

  MouseArea  {
    id: mouseArea
    anchors.fill: parent
    onClicked: container.clicked();
  }

  Component {
    id: buttonLabel
    Text {
      text: container.text
    }
  }

  Component {
    id: buttonImage
    ScaledImage {
      source: if (iconName) "../icons/" + {
                "go-up": "expand_less.svg",
                "select": "select_all.svg",
                "clear": "clear.svg",
                "go-previous": "chevron_left.svg",
                "go-next": "chevron_right.svg",
                "navigation-menu": "menu.svg",
                "edit": "create.svg",
                "add": "add.svg",
                "remove": "remove.svg"
              }[iconName]
    }
  }

  Loader {
    id: buttonContent
    anchors.centerIn: container
    sourceComponent: iconName ? buttonImage : buttonLabel
  }
}

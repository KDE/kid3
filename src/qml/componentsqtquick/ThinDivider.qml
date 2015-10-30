/**
 * \file ThinDivier.qml
 * Dummy divider line.
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
  anchors {
    left: parent.left
    right: parent.right
  }
  height: 2
  gradient: Gradient {
    GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.1) }
    GradientStop { position: 0.49; color: Qt.rgba(0, 0, 0, 0.1) }
    GradientStop { position: 0.5; color: Qt.rgba(1, 1, 1, 0.4) }
    GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.4) }
  }
}

/**
 * \file ScaledImage.qml
 * Image with scaled source size for smoothly scaled SVG icons.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 31 Oct 2015
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

Image {
  property int originalWidth: 0
  property int originalHeight: 0
  sourceSize.width: if (originalWidth > 0)
                      originalWidth * constants.imageScaleFactor
  sourceSize.height: if (originalHeight > 0)
                       originalHeight * constants.imageScaleFactor
  Component.onCompleted: {
    originalWidth = sourceSize.width
    originalHeight = sourceSize.height
  }
}

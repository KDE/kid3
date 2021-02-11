/**
 * \file PictureCollapsible.qml
 * Collapsible with picture.
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

Collapsible {
  text: qsTr("Picture")
  content: Item {
    width: parent.width
    height: coverArtImage.height

    Image {
      id: coverArtImage
      anchors.top: parent.top
      width: parent.width > 0 && parent.width < sourceSize.width
             ? parent.width : sourceSize.width
      fillMode: Image.PreserveAspectFit
      source: app.coverArtImageId
      cache: false
    }
  }
}

/**
 * \file Page.qml
 * Page managed by page stack.
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
  property alias title: titleLabel.text
  property alias menuVisible: menuButton.visible
  default property alias contents: contentsItem.data
  property bool active: visible

  // The flickable which is used to hide the header when scrolling down.
  property Flickable flickable: null
  onFlickableChanged: {
    // Check margins
    if (header.flickable) {
      var headerHeight = header.visible ? header.height : 0
      if (flickable.topMargin !== headerHeight) {
        var previousHeaderHeight = flickable.topMargin;
        flickable.topMargin = headerHeight;
        flickable.contentY -= headerHeight - previousHeaderHeight;
      }
    }

    // Connect flickable
    if (flickable) {
      var previousContentY = flickable.contentY;
      flickable.contentYChanged.connect(function() {
        if (!flickable.atYBeginning && !flickable.atYEnd) {
          var deltaContentY = flickable.contentY - previousContentY;
          if (-header.height <= 0) {
            header.y = Math.max(-header.height,
                                Math.min(header.y - deltaContentY, 0));
          } else {
            header.y = Math.max(0, Math.min(header.y - deltaContentY,
                                            -header.height));
          }
        }
        previousContentY = flickable.contentY;
      });
      flickable.movementEnded.connect(function() {
        if (flickable && flickable.contentY < 0)
          header.show();
        else if (header.y < -header.height / 2)
          header.hide();
        else
          header.show();
      });
      flickable.interactiveChanged.connect(function() {
        if (flickable && !flickable.interactive)
          header.show();
      });
      flickable.contentHeightChanged.connect(function() {
        if (flickable && flickable.height >= flickable.contentHeight)
          header.show();
      });
    }
    header.show();
  }

  signal menuRequested(variant caller)

  color: constants.backgroundColor
  anchors.fill: parent
  Item {
    id: header

    function show() {
      enabled = Qt.binding(function() { return y === 0; }); //@QtQuick2
      y = 0;
    }

    function hide() {
      enabled = false; //@QtQuick2
      y = -height;
    }

    anchors.left: parent.left
    anchors.right: parent.right
    height: titleRow.height + divider.height
    Item {
      id: titleRow
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.margins: constants.margins
      height: constants.rowHeight
      Button {
        id: prevButton
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        transparent: true
        iconName: "go-previous"
        width: visible ? height : 0
        visible: pageStack.canPop
        onClicked: pageStack.pop()
      }
      Text {
        id: titleLabel
        font.pixelSize: constants.titlePixelSize
        font.weight: Font.DemiBold
        anchors.left: prevButton.right
        anchors.verticalCenter: parent.verticalCenter
      }
      Button {
        id: menuButton
        visible: false
        iconName: "navigation-menu"
        width: height
        onClicked: menuRequested(menuButton)
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
      }
    }
    ThinDivider {
      id: divider
      anchors.top: titleRow.bottom
      height: constants.gu(1)
    }
  }
  Item {
    id: contentsItem
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: header.bottom
    anchors.bottom: parent.bottom
  }
}

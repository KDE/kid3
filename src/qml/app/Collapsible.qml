/**
 * \file Collapsible.qml
 * Base component for collapsibles.
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
import "../componentsqtquick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu

Column {
  property alias text: label.text
  property alias checked: checkBox.checked
  property alias buttons: buttonContainer.data
  property alias content: contentContainer.content

  Rectangle {
    id: collapsibleRect

    height: constants.gu(4)
    width: parent.width

    CheckBox {
      id: checkBox
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
    }
    Label {
      id: label
      anchors.left: checkBox.right
      anchors.right: buttonContainer.left
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
      anchors.rightMargin: 0
      clip: true
    }
    Row {
      id: buttonContainer
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
      anchors.rightMargin: 0
      spacing: constants.spacing
    }
  }
  Item {
    id: contentContainer
    property Item content
    anchors.left: parent.left
    anchors.right: parent.right
    width: content ? content.width : undefined
    height: content ? content.height : undefined
    onContentChanged: {
      if (content) content.parent = contentContainer;
    }
    visible: checked
  }
}

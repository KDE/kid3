/**
 * \file Tag1Collapsible.qml
 * Collapsible with tag 1 information.
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
import "ComponentsQtQuick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

Collapsible {
  id: collapsibleV1

  function acceptEdit() {
    // Force focus lost to store changes.
    frameTableV1.currentIndex = -1
  }

  text: qsTr("Tag 1") + ": " + app.selectionInfo.tagFormatV1
  buttons: [
    Button {
      id: v1MenuButton
      iconName: "navigation-menu"
      width: height
      onClicked: constants.openPopup(v1MenuPopoverComponent, v1MenuButton)

      Component {
        id: v1MenuPopoverComponent
        ActionSelectionPopover {
          id: v1MenuPopover
          delegate: ActionSelectionDelegate {
            popover: v1MenuPopover
          }
          actions: ActionList {
            Action {
              text: qsTr("To Filename")
              onTriggered: app.getFilenameFromTags(script.toTagVersion(Frame.TagV1))
            }
            Action {
              text: qsTr("From Filename")
              onTriggered: app.getTagsFromFilename(script.toTagVersion(Frame.TagV1))
            }
            Action {
              text: qsTr("From Tag 2")
              onTriggered: app.copyV2ToV1()
            }
            Action {
              text: qsTr("Copy")
              onTriggered: app.copyTagsV1()
            }
            Action {
              text: qsTr("Paste")
              onTriggered: app.pasteTagsV1()
            }
            Action {
              text: qsTr("Remove")
              onTriggered: app.removeTagsV1()
            }
          }
        }
      }
    }
  ]

  content: ListView {
    id: frameTableV1
    enabled: app.selectionInfo.tag1Used
    clip: true
    width: parent.width
    //height: count * constants.rowHeight //@QtQuick1
    height: count ? contentHeight : 0 //@QtQuick2
    interactive: false
    model: app.frameModelV1
    delegate: FrameDelegate {
      width: frameTableV1.width
      isV1: true
    }
  }

  // workaround for QTBUG-31627
  // should work with "checked: app.selectionInfo.hasTagV1" with Qt >= 5.3
  Binding {
    target: collapsibleV1
    property: "checked"
    value: app.selectionInfo.hasTagV1
  }
}

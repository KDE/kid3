/**
 * \file TagCollapsible.qml
 * Collapsible with tag information.
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
//import Ubuntu.Components.Popups 1.0 //@Ubuntu
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3 1.0

Collapsible {
  id: collapsible
  property int tagNr
  property QtObject appTag: app.tag(tagNr)
  property QtObject selTag: app.selectionInfo.tag(tagNr)
  property bool hasFrameList: tagNr !== Frame.Tag_Id3v1

  function acceptEdit() {
    // Force focus lost to store changes.
    frameTable.currentIndex = -1
  }

  text: qsTr("Tag %1").arg(tagNr + 1) + ": " + collapsible.selTag.tagFormat
  visible: tagNr <= Frame.Tag_2 || selTag.tagUsed
  buttons: [
    Button {
      iconName: "edit"
      width: height
      onClicked: {
        appTag.frameList.selectByRow(frameTable.currentIndex)
        appTag.editFrame()
      }
      visible: hasFrameList
    },
    Button {
      iconName: "add"
      width: height
      onClicked: {
        appTag.addFrame()
      }
      visible: hasFrameList
    },
    Button {
      iconName: "remove"
      width: height
      onClicked: {
        appTag.frameList.selectByRow(frameTable.currentIndex)
        appTag.deleteFrame()
      }
      visible: hasFrameList
    },
    Button {
      id: menuButton
      iconName: "navigation-menu"
      width: height
      onClicked: constants.openPopup(menuPopoverComponent, menuButton)

      Component {
        id: menuPopoverComponent
        ActionSelectionPopover {
          id: menuPopover
          delegate: ActionSelectionDelegate {
            popover: menuPopover
          }
          actions: ActionList {
            Action {
              text: qsTr("To Filename")
              onTriggered: collapsible.appTag.getFilenameFromTags()
            }
            Action {
              text: qsTr("From Filename")
              onTriggered: collapsible.appTag.getTagsFromFilename()
            }
            Action {
              text: qsTr("To Tag %1").arg(2)
              onTriggered: app.copyTag(script.toTagNumber(tagNr),
                                       script.toTagNumber(Frame.Tag_2))
              visible: tagNr > Frame.Tag_2
            }
            Action {
              text: qsTr("From Tag %1").arg(tagNr == Frame.Tag_2 ? "1" : "2")
              onTriggered: collapsible.appTag.copyToOtherTag()
            }
            Action {
              text: qsTr("Copy")
              onTriggered: collapsible.appTag.copyTags()
            }
            Action {
              text: qsTr("Paste")
              onTriggered: collapsible.appTag.pasteTags()
            }
            Action {
              text: qsTr("Remove")
              onTriggered: collapsible.appTag.removeTags()
            }
          }
        }
      }
    }
  ]

  content: ListView {
    id: frameTable
    enabled: collapsible.selTag.tagUsed
    clip: true
    width: parent.width
    //height: count * constants.rowHeight //@QtQuick1
    height: count ? contentHeight : 0 //@QtQuick2
    interactive: false
    model: collapsible.appTag.frameModel
    delegate: FrameDelegate {
      width: frameTable.width
      tagNr: collapsible.tagNr
    }
  }

  // workaround for QTBUG-31627
  // should work with "checked: collapsible.selTag.hasTag" with Qt >= 5.3
  Binding {
    target: collapsible
    property: "checked"
    value: collapsible.selTag.hasTag
  }
}

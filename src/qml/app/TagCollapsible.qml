/**
 * \file TagCollapsible.qml
 * Collapsible with tag information.
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

import QtQuick 2.9
import QtQuick.Controls 2.2
import Kid3 1.1 as Kid3

Collapsible {
  id: collapsible
  property int tagNr
  property QtObject appTag: app.tag(tagNr)
  property QtObject selTag: app.selectionInfo.tag(tagNr)
  property bool hasFrameList: tagNr !== Kid3.Frame.Tag_Id3v1

  function acceptEdit() {
    // Force focus lost to store changes.
    frameTable.currentIndex = -1
  }

  text: qsTr("Tag %1").arg(tagNr + 1) + ": " + collapsible.selTag.tagFormat
  visible: tagNr <= Kid3.Frame.Tag_2 || selTag.tagUsed
  buttons: [
    IconButton {
      iconName: "edit"
      color: collapsible.labelColor
      width: height
      onClicked: {
        appTag.frameList.selectByRow(frameTable.currentIndex)
        appTag.editFrame()
      }
      visible: hasFrameList
    },
    IconButton {
      iconName: "add"
      color: collapsible.labelColor
      width: height
      onClicked: {
        appTag.addFrame()
      }
      visible: hasFrameList
    },
    IconButton {
      iconName: "remove"
      color: collapsible.labelColor
      width: height
      onClicked: {
        appTag.frameList.selectByRow(frameTable.currentIndex)
        appTag.deleteFrame()
      }
      visible: hasFrameList
    },
    IconButton {
      id: menuButton
      iconName: "navigation-menu"
      color: collapsible.labelColor
      width: height
      onClicked: menu.open()

      Menu {
        id: menu
        MenuItem {
          id: toFileNameMenu
          text: qsTr("To Filename")
          onTriggered: collapsible.appTag.getFilenameFromTags()
        }
        MenuItem {
          text: qsTr("From Filename")
          onTriggered: collapsible.appTag.getTagsFromFilename()
        }
        MenuItem {
          text: qsTr("To Tag %1").arg(2)
          onTriggered: app.copyTag(script.toTagNumber(tagNr),
                                   script.toTagNumber(Kid3.Frame.Tag_2))
          visible: tagNr > Kid3.Frame.Tag_2
          height: visible ? toFileNameMenu.height : 0
        }
        MenuItem {
          text: qsTr("From Tag %1").arg(tagNr == Kid3.Frame.Tag_2 ? "1" : "2")
          onTriggered: collapsible.appTag.copyToOtherTag()
        }
        MenuItem {
          text: qsTr("Copy")
          onTriggered: collapsible.appTag.copyTags()
        }
        MenuItem {
          text: qsTr("Paste")
          onTriggered: collapsible.appTag.pasteTags()
        }
        MenuItem {
          text: qsTr("Remove")
          onTriggered: collapsible.appTag.removeTags()
        }
      }
    }
  ]

  content: ListView {
    id: frameTable
    enabled: collapsible.selTag.tagUsed
    clip: true
    width: parent.width
    height: count ? contentHeight : 0
    interactive: false
    model: collapsible.appTag.frameModel
    delegate: FrameDelegate {
      height: constants.controlHeight
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

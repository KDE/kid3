import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

Collapsible {
  id: collapsibleV2

  function acceptEdit() {
    // Force focus lost to store changes.
    frameTableV2.currentIndex = -1
  }

  text: qsTr("Tag 2") + ": " + app.selectionInfo.tagFormatV2
  buttons: [
    Button {
      id: v2EditButton
      iconName: "edit"
      width: height
      onClicked: {
        app.frameList.selectByRow(frameTableV2.currentIndex)
        app.editFrame()
      }
    },
    Button {
      iconName: "add"
      width: height
      onClicked: {
        app.selectAndAddFrame()
      }
    },
    Button {
      iconName: "remove"
      width: height
      onClicked: {
        app.frameList.selectByRow(frameTableV2.currentIndex)
        app.deleteFrame()
      }
    },
    Button {
      id: v2MenuButton
      iconName: "navigation-menu"
      width: height
      onClicked: constants.openPopup(v2MenuPopoverComponent, v2MenuButton)

      Component {
        id: v2MenuPopoverComponent
        ActionSelectionPopover {
          id: v2MenuPopover
          delegate: ActionSelectionDelegate {
            popover: v2MenuPopover
          }
          actions: ActionList {
            Action {
              text: qsTr("To Filename")
              onTriggered: app.getFilenameFromTags(script.toTagVersion(Frame.TagV2))
            }
            Action {
              text: qsTr("From Filename")
              onTriggered: app.getTagsFromFilename(script.toTagVersion(Frame.TagV2))
            }
            Action {
              text: qsTr("From Tag 1")
              onTriggered: app.copyV1ToV2()
            }
            Action {
              text: qsTr("Copy")
              onTriggered: app.copyTagsV2()
            }
            Action {
              text: qsTr("Paste")
              onTriggered: app.pasteTagsV2()
            }
            Action {
              text: qsTr("Remove")
              onTriggered: app.removeTagsV2()
            }
          }
        }
      }
    }
  ]

  content: ListView {
    id: frameTableV2
    clip: true
    width: parent.width
    height: collapsibleV2.height - constants.gu(4)
    model: app.frameModelV2
    delegate: FrameDelegate {
      width: frameTableV2.width
    }
  }

  Binding {
    target: collapsibleV2
    property: "checked"
    value: app.selectionInfo.hasTagV2
  }
}

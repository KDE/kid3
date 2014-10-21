import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

Collapsible {
  id: fileCollapsible

  property alias fileName: fileNameEdit.text

  signal quitRequested()

  text: qsTr("File") + ": " + app.selectionInfo.detailInfo
  checked: true
  buttons: [
    Button {
      id: mainMenuButton
      iconName: "navigation-menu"
      width: height
      onClicked: constants.openPopup(mainMenuPopoverComponent, mainMenuButton)

      Component {
        id: mainMenuPopoverComponent
        ActionSelectionPopover {
          id: mainMenuPopover
          delegate: ActionSelectionDelegate {
            popover: mainMenuPopover
          }
          actions: ActionList {
            Action {
              text: qsTr("Apply Filename Format")
              onTriggered: app.applyFilenameFormat()
            }
            Action {
              text: qsTr("Apply Tag Format")
              onTriggered: app.applyId3Format()
            }
            Action {
              text: qsTr("Apply Text Encoding")
              onTriggered: app.applyTextEncoding()
            }
            Action {
              text: qsTr("Convert ID3v2.3 to ID3v2.4")
              onTriggered: app.convertToId3v24()
            }
            Action {
              text: qsTr("Convert ID3v2.4 to ID3v2.3")
              onTriggered: app.convertToId3v23()
            }
            Action {
              text: qsTr("Revert")
              onTriggered: app.revertFileModifications()
            }
            Action {
              text: qsTr("Save")
              onTriggered: {
                var errorFiles = app.saveDirectory()
                if (errorFiles.length > 0) {
                  console.debug("Save error:" + errorFiles)
                }
              }
            }
            Action {
              text: qsTr("Quit")
              onTriggered: fileCollapsible.quitRequested()
            }
          }
        }
      }
    }
  ]

  content: Item {
    width: parent.width
    height: fileNameEdit.height + constants.gu(2)
    Image {
      id: fileNameModifiedImage
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      width: 16
      height: 16
      source: "image://kid3/fileicon/" +
              ( app.selectionInfo.fileNameChanged ? "modified" : "null")
    }
    Label {
      id: fileNameLabel
      anchors.left: fileNameModifiedImage.right
      anchors.verticalCenter: parent.verticalCenter
      text: "Name:"
    }
    TextField {
      id: fileNameEdit
      anchors.left: fileNameLabel.right
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
      text: app.selectionInfo.fileName
    }
  }
}

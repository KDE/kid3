import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

Collapsible {
  id: fileCollapsible

  property alias fileName: fileNameEdit.text

  signal mainMenuRequested(variant caller)

  text: qsTr("File") + ": " + app.selectionInfo.detailInfo
  checked: true
  buttons: [
    Button {
      id: mainMenuButton
      iconName: "navigation-menu"
      width: height
      onClicked: fileCollapsible.mainMenuRequested(mainMenuButton)
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

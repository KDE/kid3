import QtQuick 2.2
import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu

Page {
  id: page

  function updateCurrentSelection() {
    collapsibleV1.acceptEdit()
    collapsibleV2.acceptEdit()
    app.frameModelsToTags()
    if (app.selectionInfo.singleFileSelected) {
      app.selectionInfo.fileName = collapsibleFile.fileName
    }
  }

  function currentFilePath() {
    return fileList.currentFilePath()
  }

  title: app.dirName.split("/").pop() +
         (app.modified ? qsTr(" [modified]") : "") +
         (app.filtered ? qsTr(" [filtered]") : "") +
         " - Kid3"

  Item {
    anchors.fill: parent
    FileList {
      id: fileList
      anchors.left: parent.left
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins
      width: constants.gu(44)
    }

    Item {
      id: rightSide
      anchors.left: fileList.right
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins

      FileCollapsible {
        id: collapsibleFile
        anchors.topMargin: constants.margins
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onMainMenuRequested:  constants.openPopup(mainMenuPopoverComponent,
                                                  caller)
      }

      Tag1Collapsible {
        id: collapsibleV1
        anchors.topMargin: constants.margins
        anchors.top: collapsibleFile.bottom
        anchors.left: parent.left
        anchors.right: parent.right
      }

      Tag2Collapsible {
        id: collapsibleV2
        anchors.topMargin: constants.margins
        anchors.top: collapsibleV1.bottom
        anchors.bottom: collapsiblePicture.top
        anchors.left: parent.left
        anchors.right: parent.right
      }

      PictureCollapsible {
        id: collapsiblePicture
        anchors.topMargin: constants.margins
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
      }
    }
  }
}

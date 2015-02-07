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
    id: body

    property int rightSideSpace: width - fileList.width - 3 * constants.margins

    anchors.fill: parent
    FileList {
      id: fileList

      raised: true
      onClicked: {
        if (body.state == "narrow")
          rightSide.raised = false
        raised = true
      }

      color: constants.backgroundColor
      anchors.left: parent.left
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins
      width: constants.gu(44)
    }

    RaisableRectangle {
      id: rightSide

      raised: true
      onClicked: {
        if (body.state == "narrow")
          fileList.raised = false
        raised = true
      }

      color: constants.backgroundColor
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins
      width: parent.rightSideSpace

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

    states: [
      State {
        name: "narrow"
        when: body.rightSideSpace < constants.gu(50)
        PropertyChanges {
          target: fileList
          raised: !rightSide.raised
        }
        PropertyChanges {
          target: rightSide
          width: constants.gu(50)
        }
      }
    ]
  }
}

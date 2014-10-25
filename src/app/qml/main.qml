import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

MainView {
  id: root
  objectName: "mainView"
  applicationName: "Kid3"
  automaticOrientation: true
  width: constants.gu(100)
  height: constants.gu(100)

  UiConstants {
    id: constants
  }

  FrameEditorObject {
    id: frameEditor
  }

  ScriptUtils {
    id: script
  }

  ConfigObjects {
    id: configs
  }

  FrameSelectDialog {
    id: frameSelectDialog

    onFrameSelected: frameEditor.onFrameSelectionFinished(name)

    Connections {
      target: frameEditor
      onFrameSelectionRequested: frameSelectDialog.open(frameNames)
    }
  }

  FrameEditDialog {
    id: frameEditDialog
    onFrameEdited: frameEditor.onFrameEditFinished(frame)

    Connections {
      target: frameEditor
      onFrameEditRequested: frameEditDialog.open(frame)
    }
  }

  MessageDialog {
    property bool doNotRevert: false

    signal completed(bool ok)

    id: saveModifiedDialog
    title: qsTr("Warning")
    text: qsTr("The current directory has been modified.\n" +
               "Do you want to save it?")
    onYes: {
      app.saveDirectory()
      completed(true)
    }
    onNo: {
      if (!doNotRevert) {
        app.deselectAllFiles()
        app.revertFileModifications()
      }
      completed(true)
    }
    onRejected: completed(false)

    // Open dialog if any file modified.
    // completed(ok) is signalled with false if canceled.
    function openIfModified() {
      if (app.modified && app.dirName) {
        show()
      } else {
        completed(true)
      }
    }
  }


  Page {
    id: page

    title: app.dirName.split("/").pop() + (app.modified ? " [modified]" : "") + " - Kid3"

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
          onQuitRequested: confirmedQuit()
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

  Component.onCompleted: {
    app.frameEditor = frameEditor
    app.readConfig()
    app.openDirectory(configs.fileConfig().lastOpenedFile)
  }

  Connections {
    target: app

    onConfirmedOpenDirectoryRequested: confirmedOpenDirectory(paths)
    onFileSelectionUpdateRequested: updateCurrentSelection()
    onSelectedFilesUpdated: app.tagsToFrameModels()
  }

  DropArea {                        //@QtQuick2
    anchors.fill: parent            //@QtQuick2
    onDropped: {                    //@QtQuick2
      if (drop.hasUrls) {           //@QtQuick2
        app.openDropUrls(drop.urls) //@QtQuick2
      }                             //@QtQuick2
    }                               //@QtQuick2
  }                                 //@QtQuick2

  function updateCurrentSelection() {
    collapsibleV1.acceptEdit()
    collapsibleV2.acceptEdit()
    app.frameModelsToTags()
    if (app.selectionInfo.singleFileSelected) {
      app.selectionInfo.fileName = collapsibleFile.fileName
    }
  }

  function confirmedOpenDirectory(path) {
    function openIfCompleted(ok) {
      saveModifiedDialog.completed.disconnect(openIfCompleted)
      if (ok) {
        app.openDirectory(path)
      }
    }

    updateCurrentSelection()
    saveModifiedDialog.doNotRevert = false
    saveModifiedDialog.completed.connect(openIfCompleted)
    saveModifiedDialog.openIfModified()
  }

  function confirmedQuit() {
    updateCurrentSelection()
    saveModifiedDialog.doNotRevert = true
    saveModifiedDialog.completed.connect(quitIfCompleted)
    saveModifiedDialog.openIfModified()
  }

  function quitIfCompleted(ok) {
    saveModifiedDialog.completed.disconnect(quitIfCompleted)
    if (ok) {
      var currentFile = fileList.currentFilePath()
      if (currentFile) {
        configs.fileConfig().lastOpenedFile = currentFile
      }
      app.saveConfig()
      Qt.quit()
    }
  }
}

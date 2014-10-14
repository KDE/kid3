import QtQuick 2.2
import Ubuntu.Components 1.1
import Ubuntu.Components.Popups 1.0
import Ubuntu.Components.ListItems 1.0 as ListItems
import Kid3App 1.0

MainView {
  id: root
  objectName: "mainView"
  applicationName: "Kid3"
  automaticOrientation: true
  width: units.gu(100)
  height: units.gu(100)

  QtObject {
    id: constants
    property int margins: units.gu(1)
    property int spacing: units.gu(1)
    property color errorColor: "red"
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

  function centerOnRoot(item) {
    item.x = root.x + (root.width - item.width) / 2
    item.y = root.y + (root.height - item.height) / 2
  }

  FrameSelectDialog {
    id: frameSelectDialog
    onVisibleChanged: centerOnRoot(frameSelectDialog)

    onFrameSelected: frameEditor.onFrameSelectionFinished(name)

    Connections {
      target: frameEditor
      onFrameSelectionRequested: frameSelectDialog.open(frameNames)
    }
  }

  FrameEditDialog {
    id: frameEditDialog
    onVisibleChanged: centerOnRoot(frameEditDialog)
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
    onVisibleChanged: centerOnRoot(saveModifiedDialog)
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


      Item {
        id: leftSide

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: statusLabel.top
        anchors.margins: constants.margins
        width: units.gu(44)

        Row {
          id: fileButtonRow
          anchors.left: leftSide.left
          anchors.top: leftSide.top
          spacing: constants.spacing
          Button {
            id: parentDirButton
            iconName: "go-up"
            width: height
            onClicked: confirmedOpenDirectory(
                         script.getIndexRoleData(fileModel.parentModelIndex(),
                                                 "filePath"))
          }
          Button {
            iconName: "select"
            width: height
            onClicked: app.selectAllFiles()
          }
          Button {
            iconName: "clear"
            width: height
            onClicked: app.deselectAllFiles()
          }
          Button {
            iconName: "go-previous"
            width: height
            onClicked: app.previousFile()
          }
          Button {
            iconName: "go-next"
            width: height
            onClicked: app.nextFile()
          }
        }

        ListView {
          id: fileList

          anchors.left: leftSide.left
          anchors.top: fileButtonRow.bottom
          anchors.bottom: leftSide.bottom
          anchors.right: leftSide.right
          anchors.margins: constants.margins
          clip: true

          model: CheckableListModel {
            id: fileModel
            sourceModel: app.fileProxyModel
            selectionModel: app.fileSelectionModel
            rootIndex: app.fileRootIndex
            onCurrentRowChanged: {
              fileList.currentIndex = row
            }
          }

          delegate: ListItems.Standard {
            id: fileDelegate
            progression: isDir
            onClicked: {
              if (!isDir) {
                ListView.view.currentIndex = index
                fileModel.currentRow = index
              } else {
                confirmedOpenDirectory(filePath)
              }
            }
            selected: ListView.isCurrentItem
            Row {
              anchors.fill: parent

              CheckBox {
                id: checkField
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                  // QTBUG-7932, assigning is not possible
                  fileModel.setDataValue(index, "checkState",
                                         checked ? Qt.Checked : Qt.Unchecked)
                }
              }
              Binding {
                // workaround for QTBUG-31627
                // should work with "checked: checkState === Qt.Checked"
                target: checkField
                property: "checked"
                value: checkState === Qt.Checked
              }
              Rectangle {
                id: fileImage
                anchors.verticalCenter: parent.verticalCenter
                color: truncated ? constants.errorColor : "transparent"
                width: 16
                height: 16
                Image {
                  anchors.fill: parent
                  source: "image://kid3/fileicon/" + iconId
                }
              }
              Label {
                id: fileText
                anchors.verticalCenter: parent.verticalCenter
                text: fileName
                color: selected
                       ? UbuntuColors.orange : Theme.palette.selected.backgroundText
              }
            }
          }

          Connections {
            target: app
            onFileSelectionUpdateRequested: {
              // Force focus lost to store changes.
              frameTableV1.currentIndex = -1
              frameTableV2.currentIndex = -1
              app.frameModelsToTags()
              if (app.selectionInfo.singleFileSelected) {
                app.selectionInfo.fileName = fileNameEdit.text
              }
            }
            onSelectedFilesUpdated: app.tagsToFrameModels()
          }

        }
      }
      Item {
        id: rightSide
        anchors.left: leftSide.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: statusLabel.top
        anchors.margins: constants.margins

        Collapsible {
          id: collapsibleFile
          anchors.topMargin: constants.margins
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.right: parent.right
          text: qsTr("File") + ": " + app.selectionInfo.detailInfo
          checked: true
          buttons: [
            Button {
              id: mainMenuButton
              iconName: "navigation-menu"
              width: height
              onClicked: PopupUtils.open(mainMenuPopoverComponent, mainMenuButton)

              Component {
                id: mainMenuPopoverComponent
                ActionSelectionPopover {
                  id: mainMenuPopover
                  delegate: ListItems.Standard {
                    text: action.text
                    onClicked: PopupUtils.close(mainMenuPopover)
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
                      onTriggered: {
                        saveModifiedDialog.doNotRevert = true
                        saveModifiedDialog.completed.connect(quitIfCompleted)
                        saveModifiedDialog.openIfModified()
                      }
                    }
                  }
                }
              }
            }
          ]

          content: Item {
            width: parent.width
            height: fileNameEdit.height + units.gu(2)
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

        Collapsible {
          id: collapsibleV1
          anchors.topMargin: constants.margins
          anchors.top: collapsibleFile.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          text: qsTr("Tag 1") + ": " + app.selectionInfo.tagFormatV1
          buttons: [
            Button {
              id: v1MenuButton
              iconName: "navigation-menu"
              width: height
              onClicked: PopupUtils.open(v1MenuPopoverComponent, v1MenuButton)

              Component {
                id: v1MenuPopoverComponent
                ActionSelectionPopover {
                  id: v1MenuPopover
                  delegate: ListItems.Standard {
                    text: action.text
                    onClicked: PopupUtils.close(v1MenuPopover)
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
            height: units.gu(43)
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

        Collapsible {
          id: collapsibleV2
          anchors.topMargin: constants.margins
          anchors.top: collapsibleV1.bottom
          anchors.bottom: collapsiblePicture.top
          anchors.left: parent.left
          anchors.right: parent.right
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
              onClicked: PopupUtils.open(v2MenuPopoverComponent, v2MenuButton)

              Component {
                id: v2MenuPopoverComponent
                ActionSelectionPopover {
                  id: v2MenuPopover
                  delegate: ListItems.Standard {
                    text: action.text
                    onClicked: PopupUtils.close(v2MenuPopover)
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
            height: collapsibleV2.height - units.gu(4)
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

        Collapsible {
          id: collapsiblePicture
          anchors.topMargin: constants.margins
          anchors.bottom: parent.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          text: qsTr("Picture")
          content: Item {
            id: sectionPicture
            width: parent.width
            height: 120

            Image {
              id: coverArtImage
              anchors.top: parent.top
              width: 120
              sourceSize.width: 120
              sourceSize.height: 120
              source: app.coverArtImageId
              cache: false
            }
          }
        }
      }

      Text {
        id: statusLabel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: "Ready."
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
  }

  DropArea {
    anchors.fill: parent
    onDropped: {
      if (drop.hasUrls) {
        app.openDropUrls(drop.urls)
      }
    }
  }

  function confirmedOpenDirectory(path) {
    function openIfCompleted(ok) {
      saveModifiedDialog.completed.disconnect(openIfCompleted)
      if (ok) {
        app.openDirectory(path)
      }
    }

    saveModifiedDialog.doNotRevert = false
    saveModifiedDialog.completed.connect(openIfCompleted)
    saveModifiedDialog.openIfModified()
  }

  function quitIfCompleted(ok) {
    saveModifiedDialog.completed.disconnect(quitIfCompleted)
    if (ok) {
      var currentFile = fileModel.getDataValue(fileModel.currentRow,
                                               "filePath")
      if (currentFile) {
        configs.fileConfig().lastOpenedFile = currentFile
      }
      app.saveConfig()
      Qt.quit()
    }
  }
}

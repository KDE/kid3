import QtQuick 1.1
import Kid3App 1.0

Rectangle {
  id: root
  visible: true
  width: 640; height: 800
  color: constants.palette.window

  QtObject {
    id: constants
    property int rowHeight: 20
    property int margins: 6
    property int spacing: 3
    property color editColor: "#fff9a8"
    property color errorColor: "red"
    property SystemPalette palette: SystemPalette {}
  }

  FrameEditorObject {
    id: frameEditor
  }

  ScriptUtils {
    id: script
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
        open()
      } else {
        completed(true)
      }
    }
  }

  Item {
    id: titleBar
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.margins: constants.margins
    height: constants.rowHeight
    Text {
      anchors.left: parent.left
      anchors.right: mainMenuButton.left
      anchors.top: parent.top
      text: app.dirName + (app.modified ? "[modified]" : "") + " - Kid3"
    }
    Button {
      id: mainMenuButton
      anchors.right: parent.right
      anchors.top: parent.top
      text: "="
      onClicked: {
        mainMenu.currentIndex = -1
        mainMenu.toggleVisible()
      }

      DropDownList {
        id: mainMenu
        anchors.right: mainMenuButton.right
        anchors.top: mainMenuButton.bottom
        width: 200
        dropDownRoot: root
        model: [
          qsTr("Apply Filename Format"),
          qsTr("Apply Tag Format"),
          qsTr("Apply Text Encoding"),
          qsTr("Convert ID3v2.3 to ID3v2.4"),
          qsTr("Convert ID3v2.4 to ID3v2.3"),
          qsTr("Revert"),
          qsTr("Save"),
          qsTr("Quit")
        ]
        onClicked: {
          switch (currentIndex) {
          case 0:
            app.applyFilenameFormat()
            break
          case 1:
            app.applyId3Format()
            break
          case 2:
            app.applyTextEncoding()
            break
          case 3:
            app.convertToId3v24()
            break
          case 4:
            app.convertToId3v23()
            break
          case 5:
            app.revertFileModifications()
            break
          case 6:
            var errorFiles = app.saveDirectory()
            if (errorFiles.length > 0) {
              console.debug("Save error:" + errorFiles)
            }
            break
          case 7:
            // Does not work if code is here.
            tryQuit()
            break
          }
        }

        function tryQuit() {
          saveModifiedDialog.doNotRevert = true
          saveModifiedDialog.completed.connect(quitIfCompleted)
          saveModifiedDialog.openIfModified()
        }

        function quitIfCompleted(ok) {
          saveModifiedDialog.completed.disconnect(quitIfCompleted)
          if (ok) {
            Qt.quit()
          }
        }
      }
    }
  }

  Item {
    id: leftSide

    anchors.left: parent.left
    anchors.top: titleBar.bottom
    anchors.bottom: statusLabel.top
    anchors.margins: constants.margins
    width: fileButtonRow.width

    Row {
      id: fileButtonRow
      anchors.left: leftSide.left
      anchors.top: leftSide.top
      spacing: constants.spacing
      Button {
        id: parentDirButton
        text: ".."
        onClicked: {
          app.openDirectory(script.getIndexRoleData(fileModel.parentModelIndex(),
                                                    "filePath"))
        }
      }
      Button {
        text: "All"
        onClicked: app.selectAllFiles()
      }
      Button {
        text: "None"
        onClicked: app.deselectAllFiles()
      }
      Button {
        text: "Prev"
        onClicked: app.previousFile()
      }
      Button {
        text: "Next"
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

      delegate: Item {
        id: fileDelegate
        width: parent.width
        height: constants.rowHeight
        CheckField {
          id: checkField
          anchors.left: parent.left
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
        Image {
          id: fileImage
          anchors.left: checkField.right
          anchors.verticalCenter: parent.verticalCenter
          width: 16
          source: "image://kid3/fileicon/" + iconId
        }
        Rectangle {
          anchors.left: fileImage.right
          anchors.right: parent.right
          height: parent.height
          color: truncated
                 ? constants.errorColor
                 : fileDelegate.ListView.isCurrentItem
                   ? constants.palette.highlight : constants.palette.window
          Text {
            id: fileText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: fileName
            color: fileDelegate.ListView.isCurrentItem
                   ? constants.palette.highlightedText : constants.palette.text
            MouseArea {
              anchors.fill: parent
              onClicked: {
                fileDelegate.ListView.view.currentIndex = index
                fileModel.currentRow = index
              }
              onDoubleClicked: {
                if (fileModel.hasModelChildren(index)) {
                  app.openDirectory(filePath)
                }
              }
            }
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
    anchors.top: titleBar.bottom
    anchors.bottom: statusLabel.top
    anchors.margins: constants.margins

    Text {
      id: fileDetailsLabel
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.margins: constants.margins
      text: qsTr("File") + ": " + app.selectionInfo.detailInfo
    }

    Rectangle {
      id: fileNameLabelRect
      anchors.top: fileDetailsLabel.bottom
      anchors.left: parent.left
      anchors.margins: constants.margins
      width: fileNameLabel.implicitWidth
      height: constants.rowHeight
      color: app.selectionInfo.fileNameChanged
             ? constants.palette.mid : constants.palette.window
      Text {
        id: fileNameLabel
        anchors.fill: parent
        text: "Name:"
      }
    }
    Rectangle {
      id: fileNameEditRect
      anchors.top: fileDetailsLabel.bottom
      anchors.left: fileNameLabelRect.right
      anchors.right: parent.right
      anchors.margins: constants.margins
      height: constants.rowHeight
      color: constants.editColor
      TextEdit {
        id: fileNameEdit
        anchors.fill: parent
        enabled: app.selectionInfo.singleFileSelected
        text: app.selectionInfo.fileName
      }
    }
    Rectangle {
      id: collapsibleV1
      anchors.topMargin: constants.margins
      anchors.top: fileNameEditRect.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      height: constants.rowHeight
      color: constants.palette.mid

      CheckBox {
        id: checkBoxV1
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: constants.margins
        text: qsTr("Tag 1") + ": " + app.selectionInfo.tagFormatV1
        // workaround for QTBUG-31627
        // should work with "checked: app.selectionInfo.hasTagV1" with Qt >= 5.3
        Binding {
          target: checkBoxV1
          property: "checked"
          value: app.selectionInfo.hasTagV1
        }
      }
      Button {
        id: v1MenuButton
        anchors.right: parent.right
        anchors.top: parent.top
        text: "="
        onClicked: {
          v1Menu.currentIndex = -1
          v1Menu.toggleVisible()
        }

        DropDownList {
          id: v1Menu
          anchors.right: v1MenuButton.right
          anchors.top: v1MenuButton.bottom
          width: 200
          dropDownRoot: root
          model: [
            qsTr("To Filename"),
            qsTr("From Filename"),
            qsTr("From Tag 2"),
            qsTr("Copy"),
            qsTr("Paste"),
            qsTr("Remove")
          ]
          onClicked: {
            switch (currentIndex) {
            case 0:
              app.getFilenameFromTags(script.toTagVersion(Frame.TagV1))
              break
            case 1:
              app.getTagsFromFilename(script.toTagVersion(Frame.TagV1))
              break
            case 2:
              app.copyV2ToV1()
              break
            case 3:
              app.copyTagsV1()
              break
            case 4:
              app.pasteTagsV1()
              break
            case 5:
              app.removeTagsV1()
              break
            }
          }
        }
      }
    }
    Item {
      id: sectionV1
      anchors.top: collapsibleV1.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.margins: constants.margins
      height: frameTableV1.height
      visible: checkBoxV1.checked
      enabled: app.selectionInfo.tag1Used

      ListView {
        id: frameTableV1
        clip: true
        anchors.top: sectionV1.top
        anchors.left: sectionV1.left
        anchors.right: sectionV1.right
        anchors.rightMargin: constants.margins
        height: 7 * constants.rowHeight
        model: app.frameModelV1
        delegate: FrameDelegate {
          width: frameTableV1.width
          isV1: true
        }
      }
    }
    Rectangle {
      id: collapsibleV2
      anchors.top: sectionV1.visible ? sectionV1.bottom : collapsibleV1.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.topMargin: 2 * constants.margins
      height: constants.rowHeight
      color: constants.palette.mid

      CheckBox {
        id: checkBoxV2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: constants.margins
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Tag 2") + ": " + app.selectionInfo.tagFormatV2
        // workaround for QTBUG-31627
        // should work with "checked: app.selectionInfo.hasTagV2" with Qt >= 5.3
        Binding {
          target: checkBoxV2
          property: "checked"
          value: app.selectionInfo.hasTagV2
        }
      }
      Row {
        anchors.right: parent.right
        anchors.top: parent.top

        Button {
          id: v2EditButton
          text: "/"
          onClicked: {
            app.frameList.selectByRow(frameTableV2.currentIndex)
            app.editFrame()
          }
        }
        Button {
          text: "+"
          onClicked: {
            app.selectAndAddFrame()
          }
        }
        Button {
          text: "-"
          onClicked: {
            app.frameList.selectByRow(frameTableV2.currentIndex)
            app.deleteFrame()
          }
        }
        Button {
          id: v2MenuButton
          text: "="
          onClicked: {
            v2Menu.currentIndex = -1
            v2Menu.toggleVisible()
          }

          DropDownList {
            id: v2Menu
            anchors.right: v2MenuButton.right
            anchors.top: v2MenuButton.bottom
            width: 200
            dropDownRoot: root
            model: [
              qsTr("To Filename"),
              qsTr("From Filename"),
              qsTr("From Tag 2"),
              qsTr("Copy"),
              qsTr("Paste"),
              qsTr("Remove")
            ]
            onClicked: {
              switch (currentIndex) {
              case 0:
                app.getFilenameFromTags(script.toTagVersion(Frame.TagV2))
                break
              case 1:
                app.getTagsFromFilename(script.toTagVersion(Frame.TagV2))
                break
              case 2:
                app.copyV1ToV2()
                break
              case 3:
                app.copyTagsV2()
                break
              case 4:
                app.pasteTagsV2()
                break
              case 5:
                app.removeTagsV2()
                break
              }
            }
          }
        }
      }
    }
    Item {
      id: sectionV2
      anchors.top: collapsibleV2.bottom
      anchors.bottom: collapsiblePicture.top
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.margins: constants.margins
      visible: checkBoxV2.checked

      ListView {
        id: frameTableV2
        clip: true
        anchors.top: sectionV2.top
        anchors.bottom: sectionV2.bottom
        anchors.left: sectionV2.left
        anchors.right: sectionV2.right
        anchors.rightMargin: constants.margins
        model: app.frameModelV2
        delegate: FrameDelegate {
          width: frameTableV2.width
          height: constants.rowHeight
        }
      }
    }
    Rectangle {
      id: collapsiblePicture
      anchors.bottom: sectionPicture.visible ? sectionPicture.top : parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.topMargin: 2 * constants.margins
      height: constants.rowHeight
      color: constants.palette.mid

      CheckBox {
        id: checkBoxPicture
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: constants.margins
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Picture")
      }
    }
    Item {
      id: sectionPicture
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.margins: constants.margins
      height: 120
      visible: checkBoxPicture.checked

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

  Text {
    id: statusLabel
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    text: "Ready."
  }

  Component.onCompleted: {
    app.frameEditor = frameEditor
    app.openDirectory("/home/urs/projects/kid3/test/testtags")
  }

  Connections {
    target: app

    onConfirmedOpenDirectoryRequested: {
      saveModifiedDialog.doNotRevert = false
      saveModifiedDialog.completed.connect(openIfCompleted)
      saveModifiedDialog.openIfModified()

      function openIfCompleted(ok) {
        saveModifiedDialog.completed.disconnect(openIfCompleted)
        if (ok) {
          app.openDirectory(paths)
        }
      }
    }
  }
}

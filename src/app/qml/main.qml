import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
//import QtQuick.Window 2.1
import Kid3App 1.0

ApplicationWindow {
  id: root
  visible: true
  width: 640; height: 800

  title: app.dirName + (app.modified ? "[modified]" : "") + " - Kid3"

  FileDialog {
    id: fileDialog
    selectFolder: true
    nameFilters: app.createFilterString()
    onAccepted: {
      app.openDirectory(app.toStringList(fileUrls))
    }
  }

  function centerOnRoot(item) {
    item.x = root.x + (root.width - item.width) / 2
    item.y = root.y + (root.height - item.height) / 2
  }

  FrameSelectDialog {
    id: frameSelectDialog
    onVisibleChanged: centerOnRoot(this)

    onFrameSelected: app.onFrameSelectionFinished(name)

    Connections {
      target: app
      onFrameSelectionRequested: frameSelectDialog.open(frameNames)
    }
  }

  FrameEditDialog {
    id: frameEditDialog
    onVisibleChanged: centerOnRoot(this)
    onFrameEdited: app.onFrameEditFinished(frame)

    Connections {
      target: app
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
    icon: StandardIcon.Warning
    standardButtons : StandardButton.Yes | StandardButton.No |
                      StandardButton.Cancel
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

  DropArea {
    anchors.fill: parent
    onDropped: {
      if (drop.hasUrls) {
        app.openDropUrls(drop.urls)
      }
    }
  }

  Action {
    id: fileOpenAction
    text: qsTr("&Open...")
    shortcut: "Ctrl+O"
    onTriggered: {
      saveModifiedDialog.doNotRevert = false
      saveModifiedDialog.completed.connect(openIfCompleted)
      saveModifiedDialog.openIfModified()
    }

    function openIfCompleted(ok) {
      saveModifiedDialog.completed.disconnect(openIfCompleted)
      if (ok) {
        fileDialog.open()
      }
    }
  }

  Action {
    id: fileSaveAction
    text: qsTr("&Save")
    shortcut: "Ctrl+S"
    onTriggered: {
      var errorFiles = app.saveDirectory()
      if (errorFiles.length > 0) {
        console.debug("Save error:" + errorFiles)
      }
    }
  }

  Action {
    id: fileRevertAction
    text: qsTr("Re&vert")
    shortcut: "Ctrl+Z"
    onTriggered: app.revertFileModifications()
  }

  Action {
    id: filePlaylistAction
    text: qsTr("&Create Playlist")
    shortcut: "Ctrl+P"
    onTriggered: app.writePlaylist()
  }

  menuBar: MenuBar {
    Menu {
      title: qsTr("&File")
      MenuItem { action: fileOpenAction }
      MenuItem { action: fileSaveAction }
      MenuItem { action: fileRevertAction }
      MenuItem {
        text: qsTr("&Quit")
        shortcut: "Ctrl+Q"

        onTriggered: {
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
    Menu {
      title: qsTr("&Edit")
      MenuItem {
        text: qsTr("Select &All")
        shortcut: "Alt+A"
        onTriggered: app.selectAllFiles()
      }
      MenuItem {
        text: qsTr("Dese&lect")
        shortcut: "Ctrl+Shift+A"
        onTriggered: app.deselectAllFiles()
      }
      MenuItem {
        text: qsTr("&Previous File")
        shortcut: "Alt+Up"
        onTriggered: app.previousFile()
      }
      MenuItem {
        text: qsTr("&Next File")
        shortcut: "Alt+Down"
        onTriggered: app.nextFile()
      }
    }
    Menu {
      title: qsTr("&Tools")
      MenuItem {
        text: qsTr("Apply &Filename Format")
        shortcut: "Ctrl+N"
        onTriggered: app.applyFilenameFormat()
      }
      MenuItem {
        text: qsTr("Apply &Tag Format")
        shortcut: "Ctrl+T"
        onTriggered: app.applyId3Format()
      }
      MenuItem {
        text: qsTr("Apply Text &Encoding")
        onTriggered: app.applyTextEncoding()
      }
      MenuItem {
        text: qsTr("Convert ID3v2.3 to ID3v2.&4")
        onTriggered: app.convertToId3v24()
      }
      MenuItem {
        text: qsTr("Convert ID3v2.4 to ID3v2.&3")
        onTriggered: app.convertToId3v23()
      }
    }
  }

  toolBar: ToolBar {
    id: mainToolBar
    width: parent.width
    Row {
      anchors.fill: parent
      spacing: 0
      ToolButton {
        action: fileOpenAction
      }
      ToolButton {
        action: fileSaveAction
      }
      ToolButton {
        action: fileRevertAction
      }
    }
  }

  SplitView {
    anchors.fill: parent
    orientation: Qt.Horizontal
    SplitView {
      orientation: Qt.Vertical
      TableView {
        id: fileList
        headerVisible: false
        selectionMode: SelectionMode.ExtendedSelection
        Layout.fillHeight: true
        TableViewColumn { role: "fileName"; title: "Name" }
        model: VisualDataModel {
          id: fileModel
          model: app.fileProxyModel
          rootIndex: app.fileRootIndex
          delegate: fileList.contentItem.delegate
        }
        itemDelegate: Row {
          Image {
            width: 16
            // The styleData.value part is here to trigger a property change
            // when the model changes. "" + is to avoid the warning
            // "Unable to assign [undefined] to QString"
            source: "image://kid3/fileicon/" +
                    (styleData.value !== "" ? "" +
                     app.getRoleData(app.fileProxyModel, styleData.row,
                                     "iconId", app.fileRootIndex) : "null")
          }
          Text {
            text: styleData.value
          }
        }

        selection.onSelectionChanged: {
          var indexes = []
          selection.forEach(function(i) {
            indexes.push(fileModel.modelIndex(i))
          })
          app.setFileSelectionIndexes(indexes)
        }

        Connections {
          target: app
          onFileSelectionChanged: {
            var rows = app.getFileSelectionRows();
            fileList.selection.clear()
            for (var i = 0; i < rows.length; ++i) {
              fileList.selection.select(rows[i], rows[i])
            }
          }
          onFileSelectionUpdateRequested: {
            app.frameModelsToTags()
            if (app.selectionInfo.singleFileSelected) {
              app.selectionInfo.fileName = fileNameEdit.text
            }
          }
          onSelectedFilesUpdated: app.tagsToFrameModels()
        }
      }
      TableView {
        id: dirList
        headerVisible: false
        TableViewColumn { role: "fileName"; title: "Name" }
        model: VisualDataModel {
          id: dirModel
          model: app.dirProxyModel
          rootIndex: app.dirRootIndex
          delegate: dirList.contentItem.delegate
        }
        onActivated: {
          app.openDirectory(app.getRoleData(app.dirProxyModel, currentRow,
                                            "filePath", app.dirRootIndex))
        }
      }
    }
    Item {
      Label {
        id: fileDetailsLabel
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: qsTr("File") + ": " + app.selectionInfo.detailInfo
      }

      Label {
        id: fileNameLabel
        anchors.top: fileDetailsLabel.bottom
        text: "Name:"
      }
      TextField {
        id: fileNameEdit
        anchors.top: fileDetailsLabel.bottom
        anchors.left: fileNameLabel.right
        anchors.right: parent.right
        enabled: app.selectionInfo.singleFileSelected
        text: app.selectionInfo.fileName
        textColor: app.selectionInfo.fileNameChanged ? "red" : "black"
      }
      CheckBox {
        id: checkBoxV1
        anchors.top: fileNameEdit.bottom
        text: qsTr("Tag 1") + ": " + app.selectionInfo.tagFormatV1
        // workaround for QTBUG-31627
        // should work with "checked: app.selectionInfo.hasTagV1" with Qt >= 5.3
        Binding {
          target: checkBoxV1
          property: "checked"
          value: app.selectionInfo.hasTagV1
        }
      }
      Item {
        id: sectionV1
        anchors.top: checkBoxV1.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: buttonsV1.height
        visible: checkBoxV1.checked
        enabled: app.selectionInfo.tag1Used

        TableView {
          id: frameTableV1
          anchors.top: sectionV1.top
          anchors.bottom: buttonsV1.bottom
          anchors.left: sectionV1.left
          anchors.right: buttonsV1.left
          headerVisible: false
          TableViewColumn {
            id: frameTableV1NameColumn
            role: "checkState"
            title: "Name"
            delegate: FrameNameDelegate { isV1: true }
          }
          TableViewColumn {
            role: "value"
            title: "Value"
            width: frameTableV1.width - frameTableV1NameColumn.width - 4
            delegate: FrameValueDelegate { isV1: true }
          }
          model: app.frameModelV1
        }
        Column {
          id: buttonsV1
          anchors.top: frameTableV1.top
          anchors.right: sectionV1.right
          width: buttonsV2.width
          Button {
            width: parent.width
            text: "To Filename"
            onClicked: app.getFilenameFromTags(app.toTagVersion(1))
          }
          Button {
            width: parent.width
            text: "From Filename"
            onClicked: app.getTagsFromFilename(app.toTagVersion(1))
          }
          Button {
            width: parent.width
            text: "From Tag 2"
            onClicked: app.copyV2ToV1()
          }
          Button {
            width: parent.width
            text: "Copy"
            onClicked: app.copyTagsV1()
          }
          Button {
            width: parent.width
            text: "Paste"
            onClicked: app.pasteTagsV1()
          }
          Button {
            width: parent.width
            text: "Remove"
            onClicked: app.removeTagsV1()
          }
        }
      }
      CheckBox {
        id: checkBoxV2
        anchors.top: sectionV1.visible ? sectionV1.bottom : checkBoxV1.bottom
        text: qsTr("Tag 2") + ": " + app.selectionInfo.tagFormatV2
        // workaround for QTBUG-31627
        // should work with "checked: app.selectionInfo.hasTagV2" with Qt >= 5.3
        Binding {
          target: checkBoxV2
          property: "checked"
          value: app.selectionInfo.hasTagV2
        }
      }
      Item {
        id: sectionV2
        anchors.top: checkBoxV2.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        visible: checkBoxV2.checked

        TableView {
          id: frameTableV2
          anchors.top: sectionV2.top
          anchors.bottom: sectionV2.bottom
          anchors.left: sectionV2.left
          anchors.right: buttonsV2.left
          headerVisible: false
          TableViewColumn {
            id: frameTableV2NameColumn
            role: "checkState"
            title: "Name"
            delegate: FrameNameDelegate { }
          }
          TableViewColumn {
            role: "value"
            title: "Value"
            width: frameTableV2.width - frameTableV2NameColumn.width - 4
            delegate: FrameValueDelegate { }
          }
          model: app.frameModelV2
        }
        Column {
          id: buttonsV2
          anchors.top: frameTableV2.top
          anchors.right: sectionV2.right
          width: coverArtImage.width
          Button {
            width: parent.width
            text: "To Filename"
            onClicked: app.getFilenameFromTags(app.toTagVersion(2))
          }
          Button {
            width: parent.width
            text: "From Filename"
            onClicked: app.getTagsFromFilename(app.toTagVersion(2))
          }
          Button {
            width: parent.width
            text: "From Tag 1"
            onClicked: app.copyV1ToV2()
          }
          Button {
            width: parent.width
            text: "Copy"
            onClicked: app.copyTagsV2()
          }
          Button {
            width: parent.width
            text: "Paste"
            onClicked: app.pasteTagsV2()
          }
          Button {
            width: parent.width
            text: "Remove"
            onClicked: app.removeTagsV2()
          }
          Button {
            width: parent.width
            text: "Edit"
            onClicked: {
              app.frameList.selectByRow(frameTableV2.currentRow)
              app.editFrame()
            }
          }
          Button {
            width: parent.width
            text: "Add"
            onClicked: {
              app.selectAndAddFrame()
            }
          }
          Button {
            width: parent.width
            text: "Delete"
            onClicked: {
              app.frameList.selectByRow(frameTableV2.currentRow)
              app.deleteFrame()
            }
          }
          Image {
            id: coverArtImage
            width: 120
            sourceSize.width: 120
            sourceSize.height: 120
            source: app.coverArtImageId
            cache: false
          }
        }
      }
    }
  }

  statusBar: StatusBar {
    Row {
      Label {
        id: statusLabel
        text: "Ready."
      }
    }
  }

  Component.onCompleted: {
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

  onClosing: {
    close.accepted = false
    saveModifiedDialog.doNotRevert = true
    saveModifiedDialog.completed.connect(quitIfCompleted)
    saveModifiedDialog.openIfModified()

    function quitIfCompleted(ok) {
      saveModifiedDialog.completed.disconnect(quitIfCompleted)
      if (ok) {
        Qt.quit()
      }
    }
  }

}

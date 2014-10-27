import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Kid3App 1.0

Page {
  id: page

  title: qsTr("Rename Directory")

  Connections {
    target: app.dirRenamer
    onActionScheduled: {
      var str = ""
      for (var i = 0; i < actionStrs.length; ++i) {
        if (i > 0)
          str += "  "
        str += actionStrs[i]
        str += "\n"
      }
      textArea.text += str
    }
  }

  Label {
    id: previewLabel
    anchors {
      left: parent.left
      right: parent.right
      top: parent.top
      margins: constants.margins
    }
    text: qsTr("Preview")
  }
  TextArea {
    id: textArea
    anchors {
      left: parent.left
      right: parent.right
      top: previewLabel.bottom
      bottom: buttonRow.top
      margins: constants.margins
    }
    readOnly: true
    selectByMouse: false
  }
  Row {
    id: buttonRow
    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      margins: constants.margins
    }
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Cancel")
      onClicked: {
        pageStack.pop()
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("OK")
      onClicked: {
        var errorMsg = app.performRenameActions()
        if (errorMsg) {
          console.debug("Rename error: " + errorMsg)
        }
        pageStack.pop()
      }
    }
  }

  onActiveChanged: {
    if (active) {
      textArea.text = ""
      app.renameDirectory(script.toTagVersion(Frame.TagV2V1),
                          configs.renDirConfig().dirFormat, false)
    }
  }
}

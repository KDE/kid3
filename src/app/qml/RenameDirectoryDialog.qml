import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Kid3App 1.0

Dialog {
  id: page
  title: qsTr("Rename Directory")
  text: qsTr("Preview")

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

  TextArea {
    id: textArea
    height: constants.gu(35)
    readOnly: true
    selectByMouse: false
  }
  Row {
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
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
        page.hide()
      }
    }
  }

  Component.onCompleted: {
    textArea.text = ""
    app.renameDirectory(script.toTagVersion(Frame.TagV2V1),
                        configs.renDirConfig().dirFormat, false)
  }
}

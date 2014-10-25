import QtQuick 2.2
import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu

Dialog {
  id: page

  property alias filePath: textField.text
  signal finished(string path)

  title: qsTr("Open")
  text: qsTr("File path")
  TextField {
    id: textField
  }

  Row {
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
        page.finished("")
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("OK")
      onClicked: {
        page.hide()
        page.finished(page.filePath)
      }
    }
  }
}

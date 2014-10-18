import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu

Dialog {
  id: page

  signal yes
  signal no
  signal rejected

  Row {
    spacing: constants.spacing
    width: parent.width - 2 * spacing
    Button {
      width: parent.width / 3
      id: yesButton
      text: qsTr("Yes")
      onClicked: {
        page.hide()
        page.yes()
      }
    }
    Button {
      width: parent.width / 3
      text: qsTr("No")
      onClicked: {
        page.hide()
        page.no()
      }
    }
    Button {
      width: parent.width / 3
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
        page.rejected()
      }
    }
  }
}

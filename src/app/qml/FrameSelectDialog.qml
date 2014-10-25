import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu

Dialog {
  id: page

  signal frameSelected(string name);

  title: qsTr("Add Frame")
  text: qsTr("Select the frame ID")

  function open(frameNames) {
    frameSelectList.model = frameNames
    page.show()
  }

  ListView {
    id: frameSelectList
    height: constants.gu(35)

    clip: true
    delegate: Standard {
      text: modelData
      selected: ListView.view.currentIndex === index
      onClicked: ListView.view.currentIndex = index
    }
  }

  Row {
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
        page.frameSelected("")
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("OK")
      onClicked: {
        page.hide()
        page.frameSelected(frameSelectList.currentItem.text)
      }
    }
  }
}

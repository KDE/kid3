import QtQuick 2.2
import Ubuntu.Components 1.1
import Ubuntu.Components.Popups 1.0
import Ubuntu.Components.ListItems 1.0 as ListItems

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
    height: units.gu(35)

    clip: true
    delegate: ListItems.Standard {
      text: modelData
      selected: ListView.view.currentIndex === index
      onClicked: ListView.view.currentIndex = index
    }
  }

  Row {
    width: parent.width
    spacing: constants.spacing
    Button {
      width: parent.width / 2
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
        page.frameSelected("")
      }
    }
    Button {
      width: parent.width / 2
      text: qsTr("OK")
      onClicked: {
        page.hide()
        page.frameSelected(frameSelectList.currentItem.text)
      }
    }
  }
}

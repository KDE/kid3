import QtQuick 1.1

Rectangle {
  id: page

  property string title
  property string text

  signal yes
  signal no
  signal rejected

  width: 400
  height: 200
  border.width: 1
  border.color: "black"
  visible: false
  z: 0

  Item {
    anchors.fill: parent
    anchors.margins: constants.margins

    Text {
      id: titleLabel
      anchors.top: parent.top
      anchors.left: parent.left
      text: title
    }
    Text {
      anchors.top: titleLabel.bottom
      anchors.left: parent.left
      text: page.text
    }

    Row {
      spacing: constants.spacing
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins

      Button {
        text: qsTr("Yes")
        onClicked: {
          page.visible = false
          page.z = 0
          page.yes()
        }
      }
      Button {
        text: qsTr("No")
        onClicked: {
          page.visible = false
          page.z = 0
          page.no()
        }
      }
      Button {
        text: qsTr("Cancel")
        onClicked: {
          page.visible = false
          page.z = 0
          page.rejected()
        }
      }
    }
  }

  function open() {
    visible = true
    page.z = 1
  }
}

import QtQuick 1.1

Dialog {
  id: page

  property string title
  property string text

  signal yes
  signal no
  signal rejected

  function reject() {
    __MessageDialog_reject()
  }

  function __MessageDialog_reject() {
    __Dialog_reject()
    page.rejected()
  }

  width: 400
  height: 200

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

    Button {
      id: yesButton
      anchors.left: parent.left
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins
      width: (parent.width - 3 * constants.margins - 2 * constants.spacing) / 3
      text: qsTr("Yes")
      onClicked: {
        close()
        page.yes()
      }
    }
    Button {
      anchors.left: yesButton.right
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins
      width: yesButton.width
      text: qsTr("No")
      onClicked: {
        close()
        page.no()
      }
    }
    Button {
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: constants.margins
      width: yesButton.width
      text: qsTr("Cancel")
      onClicked: {
        reject()
      }
    }
  }
}

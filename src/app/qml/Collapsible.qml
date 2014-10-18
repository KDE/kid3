import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu

Column {
  property alias text: label.text
  property alias checked: checkBox.checked
  property alias buttons: buttonContainer.data
  property alias content: contentContainer.content

  Rectangle {
    id: collapsibleRect

    height: constants.gu(4)
    width: parent.width

    CheckBox {
      id: checkBox
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
    }
    Label {
      id: label
      anchors.left: checkBox.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
    }
    Row {
      id: buttonContainer
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.margins: constants.margins
      spacing: constants.spacing
    }
  }
  Item {
    id: contentContainer
    property Item content
    anchors.left: parent.left
    anchors.right: parent.right
    width: content ? content.width : undefined
    height: content ? content.height : undefined
    onContentChanged: {
      if (content) content.parent = contentContainer;
    }
    visible: checked
  }
}

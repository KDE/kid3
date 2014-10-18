import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

Empty {
  id: frameDelegate

  property bool isV1: false
  property QtObject frameModel: isV1 ? app.frameModelV1 : app.frameModelV2
  property QtObject genreModel: isV1 ? app.genreModelV1 : app.genreModelV2

  selected: ListView.view.currentIndex === index
  onClicked: ListView.view.currentIndex = index

  Component {
    id: textEdit
    Item {
      TextField {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: value
        focus: true
        onAccepted: {
          focus = false
        }
        onActiveFocusChanged: {
          if (!activeFocus) {
            script.setRoleData(frameModel, index, "value", text)
          }
        }
      }
    }
  }

  Component {
    id: genreEdit
    Item {
      ComboBox {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        dropDownParent: root
        model: genreModel
        currentText: value
        currentIndex: genreModel.getRowForGenre(value)
        onCurrentIndexChanged: script.setRoleData(frameModel, index, "value",
                                                  currentText)
      }
    }
  }

  Component {
    id: valueText
    Item {
      Label {
        anchors.leftMargin: constants.margins
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: value
      }
      MouseArea {
        anchors.fill: parent
        onClicked: {
          frameDelegate.ListView.view.currentIndex = index
        }
      }
    }
  }

  Item {
    anchors.fill: parent

    CheckBox {
      id: frameEnabledCheckBox
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      onClicked: {
        // QTBUG-7932, assigning is not possible
        script.setRoleData(frameModel, index, "checkState",
                           checked ? Qt.Checked : Qt.Unchecked)
      }
      // workaround for QTBUG-31627
      // should work with "checked: checkState === Qt.Checked" with Qt >= 5.3
      Binding {
        target: frameEnabledCheckBox
        property: "checked"
        value: checkState === Qt.Checked
      }
    }
    Rectangle {
      id: frameModifiedImage
      anchors.left: frameEnabledCheckBox.right
      anchors.verticalCenter: parent.verticalCenter
      color: truncated ? constants.errorColor : "transparent"
      width: 16
      height: 16
      Image {
        anchors.fill: parent
        source: "image://kid3/fileicon/" + (modified ? "modified" : "null")
      }
    }
    Label {
      id: frameNameLabel
      anchors.left: frameModifiedImage.right
      anchors.verticalCenter: parent.verticalCenter
      width: 150
      text: name
      color: selected
             ? constants.selectedTextColor : constants.backgroundTextColor
    }

    Loader {
      anchors.left: frameNameLabel.right
      anchors.right: parent.right
      height: parent.height
      sourceComponent: !frameDelegate.ListView.isCurrentItem
                       ? valueText : frameType === Frame.FT_Genre
                         ? genreEdit : textEdit
    }

  }
}

import QtQuick 1.1
import Kid3App 1.0

Item {
  id: frameDelegate

  property bool isV1: false
  property QtObject frameModel: isV1 ? app.frameModelV1 : app.frameModelV2
  property QtObject genreModel: isV1 ? app.genreModelV1 : app.genreModelV2

  width: 300
  height: frameEnabledCheckBox.height

  Component {
    id: textEdit
    Rectangle {
      color: constants.editColor
      TextInput {
        width: parent.width
        anchors.left: parent.left
        anchors.margins: constants.margins
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
    ComboBox {
      dropDownParent: root
      model: genreModel
      currentText: value
      currentIndex: genreModel.getRowForGenre(value)
      onCurrentIndexChanged: script.setRoleData(frameModel, index, "value",
                                                currentText)
    }
  }

  Component {
    id: valueText
    Text {
      text: value
      anchors.left: if (parent) parent.left
      anchors.margins: constants.margins
      verticalAlignment: Text.AlignVCenter
      MouseArea {
        anchors.fill: parent
        onClicked: {
          frameDelegate.ListView.view.currentIndex = index
        }
      }
    }
  }

  CheckBox {
    id: frameEnabledCheckBox
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
    width: 150
    clip: true
    text: name
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
  Loader {
    anchors.left: frameEnabledCheckBox.right
    anchors.right: parent.right
    anchors.verticalCenter: parent.verticalCenter
    height: frameEnabledCheckBox.height
    sourceComponent: !frameDelegate.ListView.isCurrentItem
                     ? valueText : frameType === Frame.FT_Genre
                       ? genreEdit : textEdit
  }
}

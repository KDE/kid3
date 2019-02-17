/**
 * \file FrameDelegate.qml
 * Delegate for frame table.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015-2018  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQuick.Controls 2.2
import Kid3 1.1 as Kid3

ItemDelegate {
  id: frameDelegate

  property int tagNr
  property QtObject frameModel: app.tag(tagNr).frameModel
  property QtObject genreModel: app.tag(tagNr).genreModel

  highlighted: ListView.view.currentIndex === index
  onClicked: ListView.view.currentIndex = index

  Component {
    id: textEdit
    Item {
      TextField {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: value
        selectByMouse: true
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
    id: differentTextEdit
    Item {
      ComboBox {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        editable: true
        model: [String.fromCharCode(0x2260)].concat(
          script.getRoleData(frameModel, index, "completions"))
        onCurrentTextChanged: script.setRoleData(frameModel, index, "value",
                              currentIndex === -1 && editText || currentText)
        Component.onCompleted: {
          currentIndex = model.indexOf(value)
          if (currentIndex === -1) {
            editText = value;
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
        editable: tagNr !== Kid3.Frame.Tag_1
        textRole: "display"
        model: genreModel
        onCurrentTextChanged: script.setRoleData(frameModel, index, "value",
                     editable && currentIndex === -1 && editText || currentText)
        Component.onCompleted: {
          currentIndex = genreModel.getRowForGenre(value)
          if (currentIndex === -1 && editable) {
            editText = value;
          }
        }
      }
    }
  }

  Component {
    id: valueText
    Item {
      Label {
        anchors.leftMargin: constants.margins
        anchors.fill: parent
        text: value
        verticalAlignment: Text.AlignVCenter
        clip: true
      }
      MouseArea {
        anchors.fill: parent
        onClicked: {
          frameDelegate.ListView.view.currentIndex = index
        }
      }
    }
  }

  Rectangle {
    anchors.fill: parent
    color: highlighted ? constants.highlightColor : "transparent"

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
      width: constants.gu(2)
      height: constants.gu(2)
      Text {
        font.family: materialFont.name
        font.pixelSize: 16
        text: "M"
        color: frameNameLabel.color
        visible: modified
      }
      MouseArea {
        id: mouseArea
        anchors.fill: parent
      }
      ToolTip {
        parent: frameNameLabel
        visible: mouseArea.pressed && truncated && notice
        text: notice
      }
    }
    Label {
      id: frameNameLabel
      anchors.left: frameModifiedImage.right
      anchors.verticalCenter: parent.verticalCenter
      width: Math.min(constants.gu(11),
                   (parent.width - constants.gu(4) - 2 * constants.margins) / 2)
      text: name
      clip: true
      color: highlighted
             ? constants.highlightedTextColor : constants.textColor
    }

    Loader {
      anchors.left: frameNameLabel.right
      anchors.right: parent.right
      anchors.rightMargin: constants.margins
      height: parent.height
      sourceComponent: !frameDelegate.ListView.isCurrentItem
                       ? valueText : frameType === Kid3.Frame.FT_Genre
                         ? genreEdit : value === String.fromCharCode(0x2260)
                           ? differentTextEdit : textEdit
    }

  }
}

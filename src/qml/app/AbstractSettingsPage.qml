/**
 * \file AbstractSettingsPage.qml
 * Base component for settings page.
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

Page {
  id: page

  signal clicked(int index)
  property list<SettingsElement> model

  function activateAll() {
    for (var i = 0; i < model.length; i++) {
      if (model[i].onActivated) {
        model[i].onActivated()
      }
    }
  }

  function deactivateAll() {
    for (var i = 0; i < model.length; i++) {
      if (model[i].onDeactivated) {
        model[i].onDeactivated()
      }
    }
  }

  header: ToolBar {
    IconButton {
      id: prevButton
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      iconName: "go-previous"
      color: titleLabel.color
      width: visible ? height : 0
      visible: page.StackView.view && page.StackView.view.depth > 1
      onClicked: page.StackView.view.pop()
    }
    Label {
      id: titleLabel
      anchors.left: prevButton.right
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      clip: true
      text: page.title
    }
  }

  Item {
    anchors.fill: parent

    Component {
      id: booleanDelegate
      SettingsItem {
        text: _modelData.name
        control: CheckBox {
          checked: _modelData.value
          onClicked: _modelData.value = checked
        }
      }
    }
    Component {
      id: booleanEditDelegate
      SettingsItem {
        id: settingsItem
        text: _modelData.name
        control: RowLayout {
          IconButton {
            iconName: "edit"
            color: settingsItem.labelColor
            onClicked: _modelData.onEdit()
          }
          CheckBox {
            checked: _modelData.value
            onClicked: _modelData.value = checked
          }
        }
      }
    }
    Component {
      id: stringDelegate
      SettingsItem {
        text: _modelData.name
        control: TextField {
          width: Math.min(_modelData.width || constants.gu(40), page.width - 2 * constants.margins)
          text: _modelData.value
          selectByMouse: true
          onAccepted: {
            focus = false
          }
          onActiveFocusChanged: {
            if (!activeFocus) {
              _modelData.value = text
            }
          }
        }
      }
    }
    Component {
      id: stringSelectionDelegate
      SettingsItem {
        text: _modelData.name
        control: ComboBox {
          width: Math.min(_modelData.width || constants.gu(40), page.width - 2 * constants.margins)
          model: _modelData.dropDownModel
          onCurrentTextChanged: _modelData.value = currentText
          currentIndex: find(_modelData.value)
        }
      }
    }
    Component {
      id: stringSelectionEditDelegate
      SettingsItem {
        id: settingsItem
        text: _modelData.name
        control: RowLayout {
          width: Math.min(_modelData.width || constants.gu(40), page.width - 2 * constants.margins)
          IconButton {
            iconName: "edit"
            color: settingsItem.labelColor
            onClicked: _modelData.onEdit()
          }
          ComboBox {
            Layout.fillWidth: true
            model: _modelData.dropDownModel
            onCurrentTextChanged: _modelData.value = currentText
            currentIndex: find(_modelData.value)
          }
        }
      }
    }
    Component {
      id: numberDelegate
      SettingsItem {
        text: _modelData.name
        control: TextField {
          width: Math.min(_modelData.width || constants.gu(40), page.width - 2 * constants.margins)
          text: _modelData.value
          selectByMouse: true
          onAccepted: {
            focus = false
          }
          onActiveFocusChanged: {
            if (!activeFocus) {
              var nr = parseInt(text)
              if (!isNaN(nr)) {
                _modelData.value = nr
              }
            }
          }
        }
      }
    }
    Component {
      id: numberSelectionDelegate
      SettingsItem {
        text: _modelData.name
        control: ComboBox {
          width: Math.min(_modelData.width || constants.gu(40), page.width - 2 * constants.margins)
          currentIndex: _modelData.value
          model: _modelData.dropDownModel
          onCurrentIndexChanged: _modelData.value = currentIndex
        }
      }
    }
    Component {
      id: clickDelegate
      Standard {
        text: _modelData.name
        progression: true
        onClicked: page.clicked(_index)
      }
    }

    ListView {
      id: listView

      clip: true
      anchors.fill: parent
      model: page.model
      delegate: Loader {
        width: ListView.view.width
        property int _index: index
        property variant _modelData: modelData
        sourceComponent:
            if (typeof modelData.value === "boolean")
              if (onEdit)
                booleanEditDelegate
              else
                booleanDelegate
            else if (typeof modelData.value === "string")
              if (modelData.dropDownModel)
                if (onEdit)
                  stringSelectionEditDelegate
                else
                  stringSelectionDelegate
              else
                stringDelegate
            else if (typeof modelData.value === "number")
              if (modelData.dropDownModel)
                numberSelectionDelegate
              else
                numberDelegate
            else
              clickDelegate
      }
    }
  }
}

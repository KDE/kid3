/**
 * \file AbstractSettingsPage.qml
 * Base component for settings page.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
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

import QtQuick 2.2
import "../componentsqtquick" //@!Ubuntu
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu
//import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3 1.0

Page {
  id: page

  signal clicked(int index)
  property list<SettingsElement> model

  Item {
    anchors.fill: parent

    Component {
      id: booleanDelegate
      SettingsItem {
        text: _modelData.name
        control: CheckBox {
          id: checkField
          checked: _modelData.value
          onClicked: _modelData.value = checked
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
          dropDownParent: root
          currentText: _modelData.value
          model: _modelData.dropDownModel
          onCurrentTextChanged: _modelData.value = currentText
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
          dropDownParent: root
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
              booleanDelegate
            else if (typeof modelData.value === "string")
              if (modelData.dropDownModel)
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

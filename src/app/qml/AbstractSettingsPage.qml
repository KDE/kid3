import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu
import Kid3App 1.0

Page {
  id: page

  signal clicked(int index)
  property list<SettingsElement> model

  Item {
    anchors.fill: parent

    Component {
      id: booleanDelegate
      Standard {
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
      Standard {
        text: _modelData.name
        control: TextField {
          width: constants.gu(45)
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
      Standard {
        text: _modelData.name
        control: ComboBox {
          width: constants.gu(45)
          dropDownParent: root
          currentText: _modelData.value
          model: _modelData.dropDownModel
          onCurrentTextChanged: _modelData.value = currentText
        }
      }
    }
    Component {
      id: numberDelegate
      Standard {
        text: _modelData.name
        control: TextField {
          width: constants.gu(45)
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
      Standard {
        text: _modelData.name
        control: ComboBox {
          width: constants.gu(45)
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

      anchors.fill: parent
      model: page.model
      delegate: Loader {
        width: ListView.view.width
        height: constants.rowHeight
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

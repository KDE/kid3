import QtQuick 2.2
import QtQuick.Controls 1.1

CheckBox {
  id: frameEnabledCheckBox
  // "" + is to avoid "Unable to assign [undefined] to QString"
  text: "" + (styleData.value >= 0
              ? app.getRoleData(model, styleData.row, "name") : "")
  onClicked: {
    app.setRoleData(model, styleData.row, "checkState", checkedState)
  }
  // workaround for QTBUG-31627
  // should work with "checked: styleData.value != 0" with Qt >= 5.3
  Binding {
    target: frameEnabledCheckBox
    property: "checked"
    value: styleData.value !== 0
  }
}

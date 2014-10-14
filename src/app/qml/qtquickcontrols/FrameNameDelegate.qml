import QtQuick 2.2
import QtQuick.Controls 1.1

CheckBox {
  property bool isV1: false
  property QtObject frameModel: isV1 ? app.frameModelV1 : app.frameModelV2

  id: frameEnabledCheckBox
  // "" + is to avoid "Unable to assign [undefined] to QString"
  text: "" + (styleData.value >= 0
              ? script.getRoleData(frameModel, styleData.row, "name") : "")
  onClicked: {
    script.setRoleData(frameModel, styleData.row, "checkState", checkedState)
  }
  // workaround for QTBUG-31627
  // should work with "checked: styleData.value != 0" with Qt >= 5.3
  Binding {
    target: frameEnabledCheckBox
    property: "checked"
    value: styleData.value !== 0
  }
}

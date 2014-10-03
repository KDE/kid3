import QtQuick 2.2
import QtQuick.Controls 1.1
import Kid3App 1.0

Loader {
  property bool isV1: false
  property QtObject frameModel: isV1 ? app.frameModelV1 : app.frameModelV2
  property QtObject genreModel: isV1 ? app.genreModelV1 : app.genreModelV2

  // see qtify TrackListView.qml
  sourceComponent: script.getRoleData(frameModel, styleData.row, "frameType")
                   === Frame.FT_Genre ? genreDelegate : valueDelegate

  Component {
    id: valueDelegate
    TextField {
      text: styleData.value
      onEditingFinished: {
        script.setRoleData(frameModel, styleData.row, styleData.role, text)
      }
    }
  }

  Component {
    id: genreDelegate
    ComboBox {
      id: genreComboBox
      model: genreModel
      textRole: "display"
      editable: !isV1
      editText: styleData.value
      onCurrentTextChanged: {
        script.setRoleData(frameModel, styleData.row, styleData.role, currentText)
      }
      // Probably a bug like QTBUG-31627, should work with
      // currentIndex: model.getRowForGenre(styleData.value)
      Binding {
        target: genreComboBox
        property: "currentIndex"
        value: genreModel.getRowForGenre(styleData.value)
      }
    }
  }
}

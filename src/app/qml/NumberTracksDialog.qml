import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Kid3App 1.0

Dialog {
  id: page

  title: qsTr("Number Tracks")

  Label {
    text: qsTr("Start number:")
    width: parent.labelWidth
  }
  TextField {
    id: startNumberEdit
    text: "1"
  }
  Label {
    text: qsTr("Destination:")
    width: parent.labelWidth
  }
  ComboBox {
    id: destinationComboBox
    dropDownParent: page
    width: parent.valueWidth
    model: [ qsTr("Tag 1"),
             qsTr("Tag 2"),
             qsTr("Tag 1 and Tag 2") ]
    function getTagVersion() {
      return [ Frame.TagV1, Frame.TagV2, Frame.TagV2V1 ][currentIndex]
    }
  }
  Row {
    id: totalRow
    spacing: constants.spacing
    CheckBox {
      id: totalCheckBox
      checked: configs.tagConfig().enableTotalNumberOfTracks
    }
    Label {
      height: totalRow.height
      verticalAlignment: Text.AlignVCenter
      text: qsTr("Total number of tracks")
    }
  }
  TextField {
    id: totalEdit
  }

  Row {
    spacing: constants.spacing
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("Cancel")
      onClicked: {
        page.hide()
      }
    }
    Button {
      width: (parent.width - parent.spacing) / 2
      text: qsTr("OK")
      onClicked: {
        var startNr = parseInt(startNumberEdit.text)
        if (!isNaN(startNr)) {
          var total = totalCheckBox.checked ? parseInt(totalEdit.text) : -1
          if (isNaN(total)) {
            total = -1
          }
          app.numberTracks(startNr, total,
                       script.toTagVersion(destinationComboBox.getTagVersion()))
        }
        page.hide()
      }
    }
  }

  onVisibleChanged: if (visible) {
    totalEdit.text = app.getTotalNumberOfTracksInDir()
  }
}

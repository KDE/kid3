import QtQuick 2.2
import Kid3App 1.0

Collapsible {
  text: qsTr("Picture")
  content: Item {
    width: parent.width
    height: 120

    Image {
      id: coverArtImage
      anchors.top: parent.top
      width: 120
      sourceSize.width: 120
      sourceSize.height: 120
      source: app.coverArtImageId
      cache: false
    }
  }
}

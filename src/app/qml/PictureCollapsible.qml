import QtQuick 2.2
import Kid3App 1.0

Collapsible {
  text: qsTr("Picture")
  content: Item {
    width: parent.width
    height: coverArtImage.height

    Image {
      id: coverArtImage
      anchors.top: parent.top
      width: parent.width > 0 && parent.width < sourceSize.width
             ? parent.width : sourceSize.width
      fillMode: Image.PreserveAspectFit
      source: app.coverArtImageId
      cache: false
    }
  }
}

import QtQuick 2.2

PopupBase {
  id: page

  property alias title: titleLabel.text
  property alias text: textLabel.text
  default property alias contents: contentsColumn.data

  parent: root
  color: constants.palette.window
  border.width: 1
  border.color: "black"

  width: Math.max(contentsColumn.childrenRect.width + 2 * constants.margins,
                  400)
  height: Math.max(titleLabel.height + textLabel.height +
                   contentsColumn.childrenRect.height + 4 * constants.margins,
                   30)

  onVisibleChanged: {
    // Center dialog on root.
    page.x = root.x + (root.width - page.width) / 2
    page.y = root.y + (root.height - page.height) / 2
  }

  Item {
    id: foreground
    width: parent.width
    Label {
      id: titleLabel
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
        margins: constants.margins
      }
    }
    Label {
      id: textLabel
      anchors {
        top: titleLabel.bottom
        left: parent.left
        right: parent.right
        margins: constants.margins
      }
      wrapMode: Text.WordWrap
    }
    Column {
      id: contentsColumn
      anchors {
        top: textLabel.bottom
        left: parent.left
        right: parent.right
        margins: constants.margins
      }
      spacing: constants.spacing
      onWidthChanged: updateChildrenWidths();
      onChildrenChanged: updateChildrenWidths()
      function updateChildrenWidths() {
        for (var i = 0; i < children.length; ++i) {
          children[i].width = contentsColumn.width;
        }
      }
    }
  }
}

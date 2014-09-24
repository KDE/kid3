import QtQuick 1.1

Rectangle {
  width:400;
  height: 400;

  Rectangle {
    id:comboBox
    property variant items: ["Item 1", "Item 2", "Item 3"]
    property alias selectedItem: chosenItemText.text;
    property alias selectedIndex: listView.currentIndex;
    signal comboClicked;
    width: 100;
    height: 30;
    z: 100;
    smooth:true;

    Rectangle {
      id:chosenItem
      radius:4;
      width:parent.width;
      height:comboBox.height;
      color: "lightsteelblue"
      smooth:true;
      Text {
        anchors.top: parent.top;
        anchors.left: parent.left;
        anchors.margins: 8;
        id:chosenItemText
        text:comboBox.items[0];
        font.family: "Arial"
        font.pointSize: 14;
        smooth:true
      }

      MouseArea {
        anchors.fill: parent;
        onClicked: {
          comboBox.state = comboBox.state==="dropDown"?"":"dropDown"
        }
      }
    }

    Rectangle {
      id:dropDown
      width:comboBox.width;
      height:0;
      clip:true;
      radius:4;
      anchors.top: chosenItem.bottom;
      anchors.margins: 2;
      color: "lightgray"

      ListView {
        id:listView
        height:500;
        model: comboBox.items
        currentIndex: 0
        delegate: Item{
          width:comboBox.width;
          height: comboBox.height;

          Text {
            text: modelData
            anchors.top: parent.top;
            anchors.left: parent.left;
            anchors.margins: 5;

          }
          MouseArea {
            anchors.fill: parent;
            onClicked: {
              comboBox.state = ""
              var prevSelection = chosenItemText.text
              chosenItemText.text = modelData
              if(chosenItemText.text != prevSelection){
                comboBox.comboClicked();
              }
              listView.currentIndex = index;
            }
          }
        }
      }
    }

    Component {
      id: highlight
      Rectangle {
        width:comboBox.width;
        height:comboBox.height;
        color: "red";
        radius: 4
      }
    }

    states: State {
      name: "dropDown";
      PropertyChanges { target: dropDown; height:40*comboBox.items.length }
    }

    transitions: Transition {
      NumberAnimation { target: dropDown; properties: "height"; easing.type: Easing.OutExpo; duration: 1000 }
    }
  }
}

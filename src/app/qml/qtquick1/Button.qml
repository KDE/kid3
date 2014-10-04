import QtQuick 1.1

Rectangle  {
  id: container

  property string text: "Button"

  signal clicked

  width: buttonLabel.width + 20
  height: constants.rowHeight
  border  { width: 1; color: Qt.darker(constants.palette.button) }
  smooth: true
  radius: 4

  // color the button with a gradient
  gradient: Gradient  {
    GradientStop  {
      position: 0.0
      color:  {
        if (mouseArea.pressed)
          return constants.palette.dark
        else
          return constants.palette.light
      }
    }
    GradientStop  { position: 1.0; color: constants.palette.button }
  }

  MouseArea  {
    id: mouseArea
    anchors.fill: parent
    onClicked: container.clicked();
  }

  Text  {
    id: buttonLabel
    anchors.centerIn: container
    color: constants.palette.buttonText
    text: container.text
  }
}

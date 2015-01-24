import QtQuick 2.2

Rectangle  {
  id: container

  property string text
  property string iconName

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
    text: if (container.text)
            container.text
          else if (iconName === "go-up")
            "^"
          else if (iconName === "select")
            "*"
          else if (iconName === "clear")
            "x"
          else if (iconName === "go-previous")
            "<"
          else if (iconName === "go-next")
            ">"
          else if (iconName === "navigation-menu")
            "="
          else if (iconName === "edit")
            "/"
          else if (iconName === "add")
            "+"
          else if (iconName === "remove")
            "-"
          else
            ""
  }
}
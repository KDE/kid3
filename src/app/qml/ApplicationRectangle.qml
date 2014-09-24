// Replacement for ApplicationWindow to start from a QQuickView

import QtQuick 2.2
import QtQuick.Controls 1.1

Rectangle {
  id: root

  // todo handle closing from QQuickWindow
  signal closing

  property MenuBar menuBar: null
  property Item toolBar
  property Item statusBar

  property string title

  onToolBarChanged: { if (toolBar) { toolBar.parent = toolBarArea } }

  onStatusBarChanged: { if (statusBar) { statusBar.parent = statusBarArea } }

  onVisibleChanged: { if (visible && menuBar) { menuBar.__parentWindow = root } }

  default property alias data: contentArea.data

  color: syspal.window

  SystemPalette {id: syspal}

  Item {
      id: backgroundItem
      anchors.fill: parent

      Keys.forwardTo: menuBar ? [menuBar.__contentItem] : []

      Item {
          id: contentArea
          anchors.top: toolBarArea.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: statusBarArea.top
      }

      Item {
          id: toolBarArea
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.right: parent.right
          implicitHeight: childrenRect.height
          height: visibleChildren.length > 0 ? implicitHeight: 0
      }

      Item {
          id: statusBarArea
          anchors.bottom: parent.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          implicitHeight: childrenRect.height
          height: visibleChildren.length > 0 ? implicitHeight: 0
      }

      onVisibleChanged: if (visible && menuBar) menuBar.__parentWindow = root

      states: State {
          name: "hasMenuBar"
          when: menuBar && !menuBar.__isNative

          ParentChange {
              target: menuBar.__contentItem
              parent: backgroundItem
          }

          PropertyChanges {
              target: menuBar.__contentItem
              x: 0
              y: 0
              width: backgroundItem.width
          }

          AnchorChanges {
              target: toolBarArea
              anchors.top: menuBar.__contentItem.bottom
          }
      }
  }
}

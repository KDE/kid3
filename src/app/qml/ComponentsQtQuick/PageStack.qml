import QtQuick 2.2

Item {
  id: pageStack

  property Item currentPage: null
  property variant elements: []
  property bool canPop: elements.length > 1

  anchors.fill: parent

  function push(page) {
    var stack = elements
    if (currentPage) {
      currentPage.visible = false
    }
    if (page.createObject) {
      currentPage = page.createObject(pageStack)
    } else if (typeof page === "string") {
      currentPage = Qt.createComponent(page).createObject(pageStack);
    } else {
      currentPage = page
    }
    var pageArray = [ page, currentPage ]
    stack.push(pageArray)
    currentPage.visible = true
    elements = stack
  }

  function pop() {
    if (currentPage) {
      var stack = elements
      currentPage.visible = false
      var pageArray = stack[stack.length - 1]
      if (pageArray[0].createObject || typeof pageArray[0] === "string") {
        if (pageArray[1].destroy) {
          // Strange, only possible with QtQuick 2.
          pageArray[1].destroy()
        } else if (currentPage.destroy) {
          currentPage.destroy()
        }
      }
      stack.pop()
      if (stack.length > 0) {
        currentPage = stack[stack.length - 1][1]
        currentPage.visible = true
      } else {
        currentPage = null
      }
      elements = stack
    }
  }
}

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
    stack.push(page)
    currentPage = page
    currentPage.visible = true
    elements = stack
  }

  function pop() {
    if (currentPage) {
      var stack = elements
      currentPage.visible = false
      stack.pop()
      if (stack.length > 0) {
        currentPage = stack[stack.length - 1]
        currentPage.visible = true
      } else {
        currentPage = null
      }
      elements = stack
    }
  }
}

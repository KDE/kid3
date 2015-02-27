/**
 * \file PageStack.qml
 * Stack of pages.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

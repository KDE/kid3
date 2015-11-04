/**
 * \file UiConstants.qml
 * Constants for UI metrics.
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
//import Ubuntu.Components 1.1 //@Ubuntu
//import Ubuntu.Components.Popups 1.0 //@Ubuntu

QtObject {
  //function gu(n) {                                          //@Ubuntu
    //return units.gu(n)                                      //@Ubuntu
  //}                                                         //@Ubuntu

  //function openPopup(component, parent, params) {           //@Ubuntu
    //PopupUtils.open(component, parent, params)              //@Ubuntu
  //}                                                         //@Ubuntu

  //function closePopup(popover) {                            //@Ubuntu
    //PopupUtils.close(popover)                               //@Ubuntu
  //}                                                         //@Ubuntu

  //property int margins: gu(1)                               //@Ubuntu
  //property int spacing: gu(1)                               //@Ubuntu
  //property color errorColor: "red"                          //@Ubuntu
  //property color comboBoxColor: Theme.palette.normal.field  //@Ubuntu
  //property color selectedTextColor: UbuntuColors.orange     //@Ubuntu
  //property color backgroundTextColor: Theme.palette.selected.backgroundText //@Ubuntu
  //property color backgroundColor: Theme.palette.normal.background //@Ubuntu
  //property int rowHeight: units.gu(6)                       //@Ubuntu
  //property int controlHeight: units.gu(5)                   //@Ubuntu

  function gu(n) {                                          //@!Ubuntu
    return n * gridUnit                                     //@!Ubuntu
  }                                                         //@!Ubuntu

  function openPopup(component, parent, params) {           //@!Ubuntu
    var popover                                             //@!Ubuntu
    if (params !== undefined) {                             //@!Ubuntu
      popover = component.createObject(parent, params)      //@!Ubuntu
    } else {                                                //@!Ubuntu
      popover = component.createObject(parent)              //@!Ubuntu
    }                                                       //@!Ubuntu
    popover.show()                                          //@!Ubuntu
  }                                                         //@!Ubuntu

  function closePopup(popover) {                            //@!Ubuntu
    popover.hide()                                          //@!Ubuntu
  }                                                         //@!Ubuntu

  property int gridUnit: 8                                  //@!Ubuntu
  property int titlePixelSize: 14                           //@!Ubuntu
  property real imageScaleFactor: 1.0                       //@!Ubuntu
  property int margins: gu(1)                               //@!Ubuntu
  property int spacing: gu(1)                               //@!Ubuntu
  property color errorColor: "red"                          //@!Ubuntu
  property color comboBoxColor: palette.mid                 //@!Ubuntu
  property color selectedTextColor: palette.highlightedText //@!Ubuntu
  property color backgroundTextColor: palette.text          //@!Ubuntu
  property color backgroundColor: palette.base              //@!Ubuntu
  property color editColor: "#fff9a8"                       //@!Ubuntu
  property color borderColor: "#b2b2b2"                     //@!Ubuntu
  property color selectedBorderColor: "#a2a2a2"             //@!Ubuntu
  property color buttonColor: "#f8f8f8"                     //@!Ubuntu
  property color selectedButtonColor: "#e5e5e5"             //@!Ubuntu
  property int rowHeight: gu(6)                             //@!Ubuntu
  property int controlHeight: gu(5)                         //@!Ubuntu
  property SystemPalette palette: SystemPalette {}          //@!Ubuntu
  property int lastPopupZ: 0                                //@!Ubuntu
}

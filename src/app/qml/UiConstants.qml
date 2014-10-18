import QtQuick 2.2
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu

QtObject {
  function gu(n) {                                          //@Ubuntu
    return units.gu(n)                                      //@Ubuntu
  }                                                         //@Ubuntu

  function openPopup(component, parent) {                   //@Ubuntu
    PopupUtils.open(component, parent)                      //@Ubuntu
  }                                                         //@Ubuntu

  function closePopup(popover) {                            //@Ubuntu
    PopupUtils.close(popover)                               //@Ubuntu
  }                                                         //@Ubuntu

  property int margins: gu(1)                               //@Ubuntu
  property int spacing: gu(1)                               //@Ubuntu
  property color errorColor: "red"                          //@Ubuntu
  property color comboBoxColor: Theme.palette.normal.field  //@Ubuntu
  property color selectedTextColor: UbuntuColors.orange     //@Ubuntu
  property color backgroundTextColor: Theme.palette.selected.backgroundText //@Ubuntu
  property int rowHeight: units.gu(6)                       //@Ubuntu

  //function gu(n) {                                          //@!Ubuntu
    //return n * 8                                            //@!Ubuntu
  //}                                                         //@!Ubuntu

  //function openPopup(component, parent) {                   //@!Ubuntu
    //var popover = component.createObject(parent)            //@!Ubuntu
    //popover.setVisible(true)                                //@!Ubuntu
    //popover.x = parent.mapToItem(root, 0, 0).x - popover.width //@!Ubuntu
  //}                                                         //@!Ubuntu

  //function closePopup(popover) {                            //@!Ubuntu
    //popover.setVisible(false)                               //@!Ubuntu
  //}                                                         //@!Ubuntu

  //property int margins: gu(1)                               //@!Ubuntu
  //property int spacing: gu(1)                               //@!Ubuntu
  //property color errorColor: "red"                          //@!Ubuntu
  //property color comboBoxColor: palette.mid                 //@!Ubuntu
  //property color selectedTextColor: palette.highlightedText //@!Ubuntu
  //property color backgroundTextColor: palette.text          //@!Ubuntu
  //property color editColor: "#fff9a8"                       //@!Ubuntu
  //property int rowHeight: gu(4)                             //@!Ubuntu
  //property SystemPalette palette: SystemPalette {}          //@!Ubuntu
}

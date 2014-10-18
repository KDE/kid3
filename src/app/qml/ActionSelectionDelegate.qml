import QtQuick 2.2
//import "ComponentsQtQuick" //@!Ubuntu
import Ubuntu.Components 1.1 //@Ubuntu
import Ubuntu.Components.Popups 1.0 //@Ubuntu
import Ubuntu.Components.ListItems 1.0 //@Ubuntu

Standard {
  property Item popover
  //text: modelData.text                 //@!Ubuntu
  text: action.text                    //@Ubuntu
  //onClicked: {                         //@!Ubuntu
    //modelData.triggered()              //@!Ubuntu
    //constants.closePopup(popover)      //@!Ubuntu
  //}                                    //@!Ubuntu
  onClicked: PopupUtils.close(popover) //@Ubuntu
}

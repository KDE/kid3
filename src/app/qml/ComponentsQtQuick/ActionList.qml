import QtQuick 2.2

QtObject {
  id: actionList
  property list<QtObject> actions
  default property alias items: actionList.actions
}
